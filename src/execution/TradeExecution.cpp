#include <cpr/cpr.h>
#include <chrono>
#include <string>
#include <iostream>
#include <sstream>
#include <openssl/hmac.h>
#include "TradeExecution.hpp"

std::array<std::string, 2> getBinanceKeys() {
	std::ifstream file(".env");
	std::array<std::string, 2> keys;
	std::string line;
	if (!std::getline(file, line)) throw std::runtime_error(".env missing BINANCE_API_KEY line");
	keys[0] = line.substr(std::string("BINANCE_API_KEY=").size());
	if (!std::getline(file, line)) throw std::runtime_error(".env missing BINANCE_SECRET_KEY line");
	keys[1] = line.substr(std::string("BINANCE_SECRET_KEY=").size());
	return keys;
}

std::string hmacSha256Hex(const std::string& key, const std::string& data) {
    unsigned char* digest = HMAC(
        EVP_sha256(),
        key.c_str(), key.size(),
        reinterpret_cast<const unsigned char*>(data.c_str()), data.size(),
        nullptr, nullptr
    );

    std::ostringstream oss;
    for (int i = 0; i < 32; ++i) { // SHA256 = 32 bytes
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return oss.str();
}

int64_t getTimestampMillis() {
    return duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string sideToString(Side side) {
	if (side == Side::BUY) return "BUY";
	return "SELL";
}

void executionLoop(SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool>& running) {
	auto keys = getBinanceKeys();
	const std::string apiKey = keys[0];
	const std::string secretKey = keys[1];

	std::ofstream outputFile("data/trades.txt");
	while (running) {
		auto orderDetails = orderQueue.try_pop();
		if (!orderDetails) {  // order queue is empty
			continue;
		}
		std::string timestamp = std::to_string(getTimestampMillis());
		std::string queryString = 
			"symbol=BTCUSDT"
			"&side=" + sideToString(orderDetails->side) +
			"&type=LIMIT"
			"&timeInForce=GTC"
			"&quantity=" + std::to_string(orderDetails->volume) +
			"&price=" + std::to_string(orderDetails->price) +
			"&timestamp=" + timestamp;

		std::string signature = hmacSha256Hex(secretKey, queryString);
		cpr::Response r = cpr::Post(
			cpr::Url{"https://testnet.binance.vision/api/v3/order?" + queryString + "&signature=" + signature},
			cpr::Header{{"X-MBX-APIKEY", apiKey}}
		);
		std::string data = r.text;
		outputFile << data << '\n';
		std::cout << "TRADE EXECUTED" << '\n';
	}
	outputFile.close();
}
