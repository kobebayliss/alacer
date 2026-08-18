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

int main() {
	OrderBook ob{"BTCUSDT"};
	build_initial_orderbook(ob, 100);
	WebSocketClient ws("wss://stream.testnet.binance.vision/ws/btcusdt@depth@100ms");
	SPSCQueue<RawMessage, CAPACITY> queue{};
	std::atomic<bool> running = true;

	std::thread consumer(bookUpdater, std::ref(queue), std::ref(ob), std::ref(running)); 
	std::thread display(OrderBookDisplay::printLoop, std::ref(ob), std::ref(running));
	ws.openConnection(queue);
	std::cin.get();

	running = false;
	ws.closeConnection();
	consumer.join();
	return 0;
}
