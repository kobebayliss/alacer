#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <string>
#include <functional>
#include <atomic>
#include "book/OrderBookBuilder.hpp"
#include "book/OrderBook.hpp"
#include "book/OrderBookDisplay.hpp"
#include "feed/WebSocketClient.hpp"
#include "feed/BookUpdater.hpp"
#include <future>

int main() {
	OrderBook ob{"BTCUSDT"};
 	WebSocketClient ws("wss://stream.binance.com:9443/ws/btcusdt@depth@100ms");
	SPSCQueue<RawMessage, CAPACITY> queue{};
	ws.openConnection(queue);
	while (!ws.isConnected()) {
		std::this_thread::yield();
	}

	uint64_t last_update_id = build_initial_orderbook(ob, 100);
	std::atomic<bool> running = true;
	auto i1 = std::async(std::launch::async, bookUpdater, std::ref(queue), std::ref(ob), std::ref(running), last_update_id); 
	std::thread display(OrderBookDisplay::printLoop, std::ref(ob), std::ref(running));
	std::cin.get();

	running = false;
	ws.closeConnection();
	std::cout << "producer writes: " << ws.produced << '\n';
	std::cout << "consumer reads: " << i1.get() << '\n';
	return 0;
}
