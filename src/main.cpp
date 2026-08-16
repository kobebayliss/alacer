#include "book/OrderBook.hpp"
#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include "book/OrderBookBuilder.hpp"
#include <string>
#include "book/OrderBookDisplay.hpp"

int main() {
	OrderBook ob = build_initial_orderbook("BTCUSDT", 100);
	OrderBookDisplay::print(ob);
	OrderBookDisplay::printTopOfBook(ob);
	ix::WebSocket webSocket;
	std::string url = "wss://stream.testnet.binance.vision/ws/btcusdt@depth@100ms";
	webSocket.setUrl(url);
	webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg)
		{
			if (msg->type == ix::WebSocketMessageType::Message)
			{
				std::cout << "received message: " << msg->str << std::endl;
				std::cout << "> " << std::flush;
			}
			else if (msg->type == ix::WebSocketMessageType::Open)
			{
				std::cout << "Connection established" << std::endl;
				std::cout << "> " << std::flush;
			}
			else if (msg->type == ix::WebSocketMessageType::Error)
			{
				// Maybe SSL is not configured properly
				std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
				std::cout << "> " << std::flush;
			}
		});
	std::cout << "Connecting to " << url << "..." << std::endl;
	webSocket.start();
	std::cin.get();
	webSocket.stop();
	return 0;
}
