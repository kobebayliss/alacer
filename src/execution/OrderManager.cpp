#include <cpr/cpr.h>
#include <iostream>
#include <string>
#include "OrderManager.hpp"
#include "../feed/BinanceAuth.hpp"

std::string sideToString(Side side) {
	if (side == Side::BUY) return "BUY";
	return "SELL";
}

// replace OrderIntent with shape for updates from binance
void orderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderIntent, CAPACITY>& sendQueue, SPSCQueue<OrderIntent, CAPACITY>& updateQueue, std::atomic<bool>& running) {
	while (running) {
		// check fills/rejections from binance
		// checks for acknowledgements outgoing orders were success
		// place new orders from intent queue
		auto orderDetails = intentQueue.try_pop();
		if (!orderDetails) {
			continue; // empty
		}
		// decide if we still want to execute order
		// hands off order details to network sender thread to not block order manager
		if (!sendQueue.try_push(*orderDetails)) {
			std::cout << "send queue is full.\n";
		}
	}
}
