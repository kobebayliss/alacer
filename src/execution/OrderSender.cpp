#include <fstream>
#include <cpr/cpr.h>
#include <iostream>
#include "OrderSender.hpp"
#include "../feed/BinanceAuth.hpp"

std::string sideToString(Side side) {
	if (side == Side::BUY) return "BUY";
	return "SELL";
}

void orderSender(SPSCQueue<OrderRequest, CAPACITY>& sendQueue, std::atomic<bool>& running) {
	std::ofstream outputFile("data/trades.txt");
	while (running) {
		auto orderDetails = sendQueue.try_pop();
		if (!orderDetails) {
			continue; // empty
		}
		const OrderIntent& intent = orderDetails->intent;
		std::string queryString = 
			"symbol=BTCUSDT"
			"&side=" + sideToString(intent.side) +
			"&type=LIMIT"
			"&timeInForce=GTC"
			"&quantity=" + std::to_string(intent.volume) +
			"&price=" + std::to_string(intent.price) +
			"&newClientOrderId=" + std::string(orderDetails->clientOrderId) +
			"&timestamp=" + std::to_string(getTimestampMillis());

		std::string signature = generateUserDataSignature(queryString);
		cpr::Response r = cpr::Post(
			cpr::Url{"https://testnet.binance.vision/api/v3/order?" + queryString + "&signature=" + signature},
			cpr::Header{{"X-MBX-APIKEY", getBinanceKeys()[0]}}
		);
		std::string data = r.text;
		outputFile << data << '\n';
	}
	outputFile.close();
}
