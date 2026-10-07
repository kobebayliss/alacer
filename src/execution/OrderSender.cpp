#include "OrderSender.hpp"

void orderSender(SPSCQueue<OrderIntent, CAPACITY>& sendQueue, std::atomic<bool>& running) {
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
