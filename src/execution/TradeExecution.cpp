#include <cpr/cpr.h>
#include <chrono>
#include <string>
#include <iostream>
#include "TradeExecution.hpp"

int64_t getTimestampMillis() {
    return duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string sideToString(Side side) {
	if (side == Side::BUY) return "BUY";
	return "SELL";
}

void executionLoop(SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool>& running) {
	while (running) {
		auto orderDetails = orderQueue.try_pop();
		if (!orderDetails) {  // order queue is empty
			continue;
		}
		cpr::Response r = cpr::Post(
			cpr::Url{"https://testnet.binance.vision/api/v3/order"},
			cpr::Parameters{
				{"symbol", "BTCUSDT"},
				{"side", sideToString(orderDetails->side)},
				{"type", "MARKET"},
				{"timestamp", std::to_string(getTimestampMillis())},
				{"price", std::to_string(orderDetails->price)},
				{"quantity", std::to_string(orderDetails->volume)}
			}
		);
		std::string data = r.text;
		std::cout << data << '\n';
		std::exit(1);
	}
}
