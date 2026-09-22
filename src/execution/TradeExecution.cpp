#include <cpr/cpr.h>
#include <iostream>
#include <string>
#include "TradeExecution.hpp"
#include "../feed/BinanceAuth.hpp"

std::string sideToString(Side side) {
	if (side == Side::BUY) return "BUY";
	return "SELL";
}

void executionLoop(SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool>& running, std::atomic<OrderStatus>& orderStatus) {
	std::ofstream outputFile("data/trades.txt");
	while (running) {
		auto orderDetails = orderQueue.try_pop();
		if (!orderDetails) {  // order queue is empty
			std::this_thread::sleep_for(std::chrono::microseconds(100));
			continue;
		}
		// adjust so we are not reconstructing this signature each time? only change timestamp, price, quantity
		std::string queryString = 
			"symbol=BTCUSDT"
			"&side=" + sideToString(orderDetails->side) +
			"&type=LIMIT"
			"&timeInForce=GTC"
			"&quantity=" + std::to_string(orderDetails->volume) +
			"&price=" + std::to_string(orderDetails->price) +
			"&timestamp=" + std::to_string(getTimestampMillis());

		std::string signature = generateUserDataSignature(queryString);
		cpr::Response r = cpr::Post(
			cpr::Url{"https://testnet.binance.vision/api/v3/order?" + queryString + "&signature=" + signature},
			cpr::Header{{"X-MBX-APIKEY", getBinanceKeys()[0]}}
		);
		std::string data = r.text;
		outputFile << data << '\n';
		std::cout << "TRADE EXECUTED" << '\n';
	}
	outputFile.close();
}
