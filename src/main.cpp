#include <chrono>
#include <cpr/parameters.h>
#include <ixwebsocket/IXWebSocket.h>
#include <cpr/cpr.h>
#include <rapidjson/document.h>
#include <iostream>
#include <string>
#include <functional>
#include <atomic>
#include <future>
#include <thread>
#include <utility>
#include "backtesting/BacktestDataFeed.hpp"
#include "book/OrderBookBuilder.hpp"
#include "book/OrderBook.hpp"
#include "book/OrderBookDisplay.hpp"
#include "feed/BinanceAuth.hpp"
#include "feed/EventHandler.hpp"
#include "ixwebsocket/IXWebSocketMessage.h"
#include "types/SPSCQueue.hpp"
#include "types/RawMessage.hpp"
#include "types/OrderTypes.hpp"
#include "types/WebSocketClient.hpp"
#include "feed/BookUpdater.hpp"
#include "execution/Strategy.hpp"
#include "execution/OrderSender.hpp"
#include "execution/OrderManager.hpp"

const bool BACKTESTING_MODE = false;

// wake threads waiting on updated
static void signalShutdown(OrderBook& ob, std::atomic<bool>& running) {
	running = false;
	ob.updated.store(true, std::memory_order_release);
	ob.updated.notify_all();
}

static void runBacktest(OrderBook& ob, SPSCQueue<RawMessage, CAPACITY>& eventQueue, SPSCQueue<OrderIntent, CAPACITY>& intentQueue) {
	FILE* dataFile = fopen("marketdata.json", "rb");
	if (!dataFile) {
		std::cerr << "Error: could not open backtest data file\n";
		return;
	}

	std::string bookData = *getBookData(dataFile).get();
	uint64_t lastUpdateId = build_initial_orderbook(ob, bookData);

	std::atomic<bool> running = true;
	std::atomic<PositionView> positionView{PositionView{}};
	auto producer = std::async(std::launch::async, backtestJsonFile, std::ref(dataFile), std::ref(eventQueue));
	auto consumer = std::async(std::launch::async, bookUpdater, std::ref(eventQueue), std::ref(ob), std::ref(running), lastUpdateId, static_cast<std::ofstream*>(nullptr));
	std::thread strategy(strategyLoop, std::ref(ob), std::ref(intentQueue), std::ref(positionView), std::ref(running));

	std::cin.get();
	signalShutdown(ob, running);
	strategy.join();

	std::cout << "producer writes: " << producer.get() << '\n';
	std::cout << "consumer reads: "  << consumer.get() << '\n';

	fclose(dataFile);
}

static void runLive(OrderBook& ob, SPSCQueue<RawMessage, CAPACITY>& eventQueue, SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderRequest, CAPACITY>& sendQueue, SPSCQueue<OrderUpdate, CAPACITY>& updateQueue) {
	WebSocketClient depthStream("wss://stream.testnet.binance.vision/ws/btcusdt@depth@100ms", "data/marketdata.json", 
		[&eventQueue](const ix::WebSocketMessagePtr& msg) {
			EventHandler::handleDepthUpdate(msg, eventQueue);
		}
	);
	bool subscribed = false;
	WebSocketClient* userDataStreamPtr = nullptr;
	std::ofstream outputFile("data/updates.txt");
	WebSocketClient userDataStream("wss://ws-api.testnet.binance.vision:443/ws-api/v3", "data/userdata.json",
		[&updateQueue, &subscribed, &userDataStreamPtr, &outputFile](const ix::WebSocketMessagePtr& msg) {
			if (subscribed) {
				EventHandler::handleUserDataUpdate(msg, updateQueue, outputFile);
			} else {
				if (msg->type == ix::WebSocketMessageType::Message) {
					rapidjson::Document doc;
					doc.Parse(msg->str.c_str());
					if (!doc.HasParseError() && doc.HasMember("status") && doc["status"].GetInt() == 200) {
						userDataStreamPtr->sendMessage(generateUserStreamRequest());
						subscribed = true;
						std::cout << "Logged on, subscribed to user data stream\n";
					}
				}
			}
		}
	);
	userDataStreamPtr = &userDataStream;

	std::thread connectDepthStream([&]() {
		depthStream.openConnection();
		depthStream.start();
	});
	std::thread connectUserDataStream([&]() {
		userDataStream.openConnection();
		userDataStream.start();
		while (!userDataStream.isConnected()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // spin to avoid race cond
		}
		userDataStream.sendMessage(generateSessionLogonRequest());
	});
	while (!depthStream.isConnected() || !userDataStream.isConnected()) {
		std::this_thread::yield();
	}
	connectDepthStream.join();
	connectUserDataStream.join();

	std::string snapshotJson;
	uint64_t lastUpdateId = build_initial_orderbook(ob, snapshotJson);
	depthStream.writeSnapshot(snapshotJson);

	std::atomic<bool> running = true;
	std::atomic<PositionView> positionView{PositionView{}};
	auto consumer = std::async(std::launch::async, bookUpdater, std::ref(eventQueue), std::ref(ob), std::ref(running), lastUpdateId, &depthStream.outputFile);
	// std::thread display(OrderBookDisplay::printLoop, std::ref(ob), std::ref(running));
	std::thread strategy(strategyLoop, std::ref(ob), std::ref(intentQueue), std::ref(positionView), std::ref(running));
	std::thread manager(orderManager, std::ref(intentQueue), std::ref(sendQueue), std::ref(updateQueue), std::ref(positionView), std::ref(running));
	std::thread sender(orderSender, std::ref(sendQueue), std::ref(running));

	std::cin.get();
	depthStream.closeConnection();
	userDataStream.closeConnection();
	signalShutdown(ob, running);
	strategy.join();
	// display.join();
	manager.join();
	sender.join();

	std::cout << "producer writes: " << depthStream.produced << '\n';
	std::cout << "consumer reads: "  << consumer.get() << '\n';
}

static void cancelAllOpenOrders() {
	std::string queryString =
		"symbol=BTCUSDT"
		"&timestamp=" + std::to_string(getTimestampMillis());
	std::string signature = generateUserDataSignature(queryString);

	cpr::Response r = cpr::Delete(
		cpr::Url{"https://testnet.binance.vision/api/v3/openOrders?" + queryString + "&signature=" + signature},
		cpr::Header{{"X-MBX-APIKEY", getBinanceKeys()[0]}}
	);

	if (r.status_code == 200) {
		std::cout << "Cancelled all open orders\n";
	} else {
		std::cerr << "Cancel failed " << r.status_code << " " << r.text << std::endl;
	}
}

static void getExchangeInfo() {
	cpr::Response r = cpr::Get(cpr::Url{"https://testnet.binance.vision/api/v3/exchangeInfo?symbol=BTCUSDT"});
	std::cout << r.status_code << '\n' << r.text << '\n';
}

static bool fetchAccount(rapidjson::Document& doc) {
	std::string queryString = "omitZeroBalances=true&timestamp=" + std::to_string(getTimestampMillis());
	std::string signature = generateUserDataSignature(queryString);
	cpr::Response r = cpr::Get(cpr::Url{
	    "https://testnet.binance.vision/api/v3/account?" + queryString + "&signature=" + signature}, cpr::Header{{"X-MBX-APIKEY", getBinanceKeys()[0]}}
	);
	if (r.status_code != 200) {
		std::cerr << "Failed to fetch account summary: " << r.status_code << " " << r.text << '\n';
		return false;
	}
	doc.Parse(r.text.c_str());
	if (doc.HasParseError()) {
		std::cerr << "Failed to parse account summary\n";
		return false;
	}
	return true;
}

static std::pair<std::pair<double, double>, std::pair<double, double>> getBalances() {  // returns balances of BTC and USDT (free, locked)
	rapidjson::Document doc;
	if (!fetchAccount(doc)) return {};

	int found = 0;
	std::pair<std::pair<double, double>, std::pair<double, double>> balances;
	for (const auto& balance : doc["balances"].GetArray()) {
		std::string asset = balance["asset"].GetString();
		if (asset != "BTC" && asset != "USDT")
			continue;

		double free = std::stod(balance["free"].GetString());
		double locked = std::stod(balance["locked"].GetString());
		auto& slot = (asset == "BTC") ? balances.first : balances.second;
		slot = std::make_pair(free, locked);
		if (++found == 2) break;
	}
	return balances;
}

static void printAccountSummary() {
	rapidjson::Document doc;
	if (!fetchAccount(doc)) return;

	std::cout << "=== Account Summary ===\n"
		<< "Can trade: "
		<< (doc["canTrade"].GetBool() ? "Yes" : "No") << '\n'
		<< "Can withdraw: "
		<< (doc["canWithdraw"].GetBool() ? "Yes" : "No") << '\n'
		<< "Can deposit: "
		<< (doc["canDeposit"].GetBool() ? "Yes" : "No") << '\n';

	int found = 0;
	for (const auto& balance : doc["balances"].GetArray()) {
		std::string asset = balance["asset"].GetString();
		if (asset != "BTC" && asset != "USDT")
			continue;

		double free = std::stod(balance["free"].GetString());
		double locked = std::stod(balance["locked"].GetString());
		std::cout << asset << " | Free: " << free << " | Locked: " << locked << '\n';
		if (++found == 2) break;
	}
}

int main() {
	cancelAllOpenOrders();
	printAccountSummary();
	auto balances = getBalances();
	// OrderBook ob{"BTCUSDT"};
	// SPSCQueue<RawMessage, CAPACITY> eventQueue{};
	// SPSCQueue<OrderIntent, CAPACITY> intentQueue{};
	// SPSCQueue<OrderRequest, CAPACITY> sendQueue{};
	// SPSCQueue<OrderUpdate, CAPACITY> updateQueue{};
	//
	// if (BACKTESTING_MODE) {
	// 	runBacktest(ob, eventQueue, intentQueue);
	// } else {
	// 	runLive(ob, eventQueue, intentQueue, sendQueue, updateQueue);
	// }
	//
	return 0;
}
