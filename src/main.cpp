#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <string>
#include <functional>
#include <atomic>
#include <future>
#include "backtesting/BacktestDataFeed.hpp"
#include "book/OrderBookBuilder.hpp"
#include "book/OrderBook.hpp"
#include "book/OrderBookDisplay.hpp"
#include "execution/TradeExecution.hpp"
#include "types/SPSCQueue.hpp"
#include "types/RawMessage.hpp"
#include "types/OrderIntent.hpp"
#include "feed/WebSocketClient.hpp"
#include "feed/BookUpdater.hpp"
#include "execution/Strategy.hpp"

const bool BACKTESTING_MODE = false;

// wake threads waiting on updated
static void signalShutdown(OrderBook& ob, std::atomic<bool>& running) {
	running = false;
	ob.updated.store(true, std::memory_order_release);
	ob.updated.notify_all();
}

static void runBacktest(OrderBook& ob, SPSCQueue<RawMessage, CAPACITY>& eventQueue, SPSCQueue<OrderIntent, CAPACITY>& orderQueue) {
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
	std::thread strategy(strategyLoop, std::ref(ob), std::ref(orderQueue), std::ref(running));

	std::cin.get();
	signalShutdown(ob, running);
	strategy.join();

	std::cout << "producer writes: " << producer.get() << '\n';
	std::cout << "consumer reads: "  << consumer.get() << '\n';

	fclose(dataFile);
}

static void runLive(OrderBook& ob, SPSCQueue<RawMessage, CAPACITY>& eventQueue, SPSCQueue<OrderIntent, CAPACITY>& orderQueue) {
	WebSocketClient ws("wss://stream.testnet.binance.vision/ws/btcusdt@depth@100ms", "data/marketdata.json");

	ws.openConnection(eventQueue);
	while (!ws.isConnected()) {
		std::this_thread::yield();
	}

	std::string snapshotJson;
	uint64_t lastUpdateId = build_initial_orderbook(ob, snapshotJson);
	ws.writeSnapshot(snapshotJson);

	std::atomic<bool> running = true;
	auto consumer = std::async(std::launch::async, bookUpdater, std::ref(eventQueue), std::ref(ob), std::ref(running), lastUpdateId, &ws.outputFile);
	std::thread display(OrderBookDisplay::printLoop, std::ref(ob), std::ref(running));
	std::thread strategy(strategyLoop, std::ref(ob), std::ref(orderQueue), std::ref(running));
	std::thread execution(executionLoop, std::ref(orderQueue), std::ref(running));

	std::cin.get();
	ws.closeConnection();
	signalShutdown(ob, running);
	strategy.join();
	execution.join();
	display.join();

	std::cout << "producer writes: " << ws.produced << '\n';
	std::cout << "consumer reads: "  << consumer.get() << '\n';
}

int main() {
	OrderBook ob{"BTCUSDT"};
	SPSCQueue<RawMessage, CAPACITY> eventQueue{};
	SPSCQueue<OrderIntent, CAPACITY> orderQueue{};
	
	if (BACKTESTING_MODE) {
		runBacktest(ob, eventQueue, orderQueue);
	} else {
		runLive(ob, eventQueue, orderQueue);
	}

	return 0;
}
