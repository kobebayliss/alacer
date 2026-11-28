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
	cpr::Header header{{"X-MBX-APIKEY", getBinanceKeys()[0]}};
	while (running) {
		auto orderDetails = sendQueue.try_pop();
		if (!orderDetails) {
			continue; // empty
		}
		const OrderIntent& intent = orderDetails->intent;
		const std::string timestamp = std::to_string(getTimestampMillis());
		cpr::Response r;
		switch (intent.type) {
			case IntentType::PLACE: {
				std::string queryString =
					"symbol=BTCUSDT"
					"&side=" + sideToString(intent.side) +
					"&type=LIMIT"
					"&timeInForce=GTC"
					"&quantity=" + std::to_string(intent.volume) +
					"&price=" + std::to_string(intent.price) +
					"&newClientOrderId=" + std::string(orderDetails->clientOrderId) +
					"&timestamp=" + timestamp;
				std::string signature = generateUserDataSignature(queryString);
				r = cpr::Post(
					cpr::Url{"https://testnet.binance.vision/api/v3/order?" + queryString + "&signature=" + signature},
					header
				);
				break;
			}
			case IntentType::CANCEL: {
				// cancels a single order by the client id we assigned when placing it
				std::string queryString =
					"symbol=BTCUSDT"
					"&origClientOrderId=" + std::string(orderDetails->clientOrderId) +
					"&timestamp=" + timestamp;
				std::string signature = generateUserDataSignature(queryString);
				r = cpr::Delete(
					cpr::Url{"https://testnet.binance.vision/api/v3/order?" + queryString + "&signature=" + signature},
					header
				);
				break;
			}
			case IntentType::CANCEL_ALL: {
				// cancels every open order on the symbol (both sides)
				std::string queryString =
					"symbol=BTCUSDT"
					"&timestamp=" + timestamp;
				std::string signature = generateUserDataSignature(queryString);
				r = cpr::Delete(
					cpr::Url{"https://testnet.binance.vision/api/v3/openOrders?" + queryString + "&signature=" + signature},
					header
				);
				break;
			}
		}
		outputFile << r.text << '\n';
	}
	outputFile.close();
}
