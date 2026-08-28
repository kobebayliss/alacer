#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <string>
#include <functional>
#include <atomic>
#include "backtesting/BacktestDataFeed.hpp"
#include "book/OrderBookBuilder.hpp"
#include "book/OrderBook.hpp"
#include "book/OrderBookDisplay.hpp"
#include "feed/WebSocketClient.hpp"
#include "feed/BookUpdater.hpp"
#include "strategy/Strategy.hpp"
#include <future>

const bool BACKTESTING_MODE = true;

int main() {
	OrderBook ob{"BTCUSDT"};
	SPSCQueue<RawMessage, CAPACITY> queue{};
	if (BACKTESTING_MODE) {
		FILE* data_file = fopen("data/marketdata4.json", "rb");
		std::string bookData = *getBookData(data_file).get();
		uint64_t last_update_id = build_initial_orderbook(ob, bookData);
		std::atomic<bool> running = true;
		auto p1 = std::async(std::launch::async, backtestJsonFile, std::ref(data_file), std::ref(queue));
		auto i1 = std::async(std::launch::async, bookUpdater, std::ref(queue), std::ref(ob), std::ref(running), last_update_id); 
		std::thread strategy(strategyLoop, std::ref(ob), std::ref(running));
		std::cin.get();

		running = false;
		ob.updated.store(true, std::memory_order_release);
		ob.updated.notify_all();
		strategy.join();
		std::cout << "producer writes: " << p1.get() << '\n';
		std::cout << "consumer reads: " << i1.get() << '\n';
	} else {
		WebSocketClient ws("wss://stream.binance.com:9443/ws/btcusdt@depth@100ms");
		ws.openConnection(queue);
		while (!ws.isConnected()) {
			std::this_thread::yield();
		}

		uint64_t last_update_id = build_initial_orderbook(ob);
		std::atomic<bool> running = true;
		auto i1 = std::async(std::launch::async, bookUpdater, std::ref(queue), std::ref(ob), std::ref(running), last_update_id); 
		std::thread display(OrderBookDisplay::printLoop, std::ref(ob), std::ref(running));
		std::thread strategy(strategyLoop, std::ref(ob), std::ref(running));
		std::cin.get();

		running = false;
		ws.closeConnection();
		ob.updated.store(true, std::memory_order_release);
		ob.updated.notify_all();
		strategy.join();
		display.join();
		std::cout << "producer writes: " << ws.produced << '\n';
		std::cout << "consumer reads: " << i1.get() << '\n';
	}
	return 0;
}
