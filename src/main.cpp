#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <string>
#include "book/OrderBookBuilder.hpp"
#include "book/OrderBook.hpp"
#include "book/OrderBookDisplay.hpp"
#include "feed/WebSocketClient.hpp"

int main() {
	OrderBook ob = build_initial_orderbook("BTCUSDT", 100);
	OrderBookDisplay::print(ob);
	OrderBookDisplay::printTopOfBook(ob);
	WebSocketClient ws("wss://stream.testnet.binance.vision/ws/btcusdt@depth@100ms");
	ws.openConnection();
	std::cin.get();
	ws.closeConnection();
	return 0;
}
