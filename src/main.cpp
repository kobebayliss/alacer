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
	auto producer = std::async(std::launch::async, backtestJsonFile, std::ref(dataFile), std::ref(eventQueue));
	auto consumer = std::async(std::launch::async, bookUpdater, std::ref(eventQueue), std::ref(ob), std::ref(running), lastUpdateId, static_cast<std::ofstream*>(nullptr));
	std::thread strategy(strategyLoop, std::ref(ob), std::ref(intentQueue), std::ref(running));

	std::cin.get();
	signalShutdown(ob, running);
	strategy.join();

	std::cout << "producer writes: " << producer.get() << '\n';
	std::cout << "consumer reads: "  << consumer.get() << '\n';

	fclose(dataFile);
}

static void runLive(OrderBook& ob, SPSCQueue<RawMessage, CAPACITY>& eventQueue, SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderRequest, CAPACITY>& sendQueue, SPSCQueue<OrderIntent, CAPACITY>& updateQueue) {
	WebSocketClient depthStream("wss://stream.testnet.binance.vision/ws/btcusdt@depth@100ms", "data/marketdata.json", 
		[&eventQueue](const ix::WebSocketMessagePtr& msg) {
			EventHandler::handleDepthUpdate(msg, eventQueue);
		}
	);
	bool subscribed = false;
	WebSocketClient* userDataStreamPtr = nullptr;
	WebSocketClient userDataStream("wss://ws-api.testnet.binance.vision:443/ws-api/v3", "data/userdata.json",
		[&updateQueue, &subscribed, &userDataStreamPtr](const ix::WebSocketMessagePtr& msg) {
			if (subscribed) {
				EventHandler::handleUserDataUpdate(msg, updateQueue);
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
	auto consumer = std::async(std::launch::async, bookUpdater, std::ref(eventQueue), std::ref(ob), std::ref(running), lastUpdateId, &depthStream.outputFile);
	std::thread display(OrderBookDisplay::printLoop, std::ref(ob), std::ref(running));
	std::thread strategy(strategyLoop, std::ref(ob), std::ref(intentQueue), std::ref(running));

	std::cin.get();
	depthStream.closeConnection();
	userDataStream.closeConnection();
	signalShutdown(ob, running);
	strategy.join();
	display.join();

	std::cout << "producer writes: " << depthStream.produced << '\n';
	std::cout << "consumer reads: "  << consumer.get() << '\n';
}

int main() {
	OrderBook ob{"BTCUSDT"};
	SPSCQueue<RawMessage, CAPACITY> eventQueue{};
	SPSCQueue<OrderIntent, CAPACITY> intentQueue{};
	SPSCQueue<OrderRequest, CAPACITY> sendQueue{};
	SPSCQueue<OrderIntent, CAPACITY> updateQueue{};

	if (BACKTESTING_MODE) {
		runBacktest(ob, eventQueue, intentQueue);
	} else {
		runLive(ob, eventQueue, intentQueue, sendQueue, updateQueue);
	}

	return 0;
}
