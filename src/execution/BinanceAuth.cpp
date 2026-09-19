#include <cpr/cpr.h>
#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include <openssl/hmac.h>
#include <string>
#include "BinanceAuth.hpp"

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


// pass in query string like "apiKey=...
std::string generateUserDataSignature(const std::string& queryString) {
	auto keys = getBinanceKeys();
	const std::string apiKey = keys[0];
	const std::string secretKey = keys[1];
	std::string signature = hmacSha256Hex(secretKey, queryString);
	// for user data stream
	// std::string queryString = "apiKey=" + apiKey + "&timestamp=" + std::to_string(timestamp);  
	// request << "{"
	// 	<< "\"id\":\"userdatastream-sub-1\","
	// 	<< "\"method\":\"userDataStream.subscribe.signature\","
	// 	<< "\"params\":{"
	// 	    << "\"apiKey\":\"" << apiKey << "\","
	// 	    << "\"timestamp\":" << timestamp << ","
	// 	    << "\"signature\":\"" << signature << "\""
	// 	<< "}"
	// 	<< "}";
	return signature;
}
