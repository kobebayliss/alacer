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
void OrderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderIntent, CAPACITY>& updateQueue, std::atomic<bool>& running) {
	while (running) {
		// check fills/rejections from binance
		// checks for acknowledgements outgoing orders were success
		// place new orders from intent queue
		auto orderDetails = intentQueue.try_pop();
	}
}

void placeOrder() {
	// hands off order details to network sender thread to not block order manager
	// to be moved to network sender thread
	// std::ofstream outputFile("data/trades.txt");
	// std::string queryString = 
	// 	"symbol=BTCUSDT"
	// 	"&side=" + sideToString(orderDetails->side) +
	// 	"&type=LIMIT"
	// 	"&timeInForce=GTC"
	// 	"&quantity=" + std::to_string(orderDetails->volume) +
	// 	"&price=" + std::to_string(orderDetails->price) +
	// 	"&timestamp=" + std::to_string(getTimestampMillis());
	//
	// std::string signature = generateUserDataSignature(queryString);
	// cpr::Response r = cpr::Post(
	// 	cpr::Url{"https://testnet.binance.vision/api/v3/order?" + queryString + "&signature=" + signature},
	// 	cpr::Header{{"X-MBX-APIKEY", getBinanceKeys()[0]}}
	// );
	// std::string data = r.text;
	// outputFile << data << '\n';
	// std::cout << "TRADE EXECUTED" << '\n';
	// outputFile.close();
}
