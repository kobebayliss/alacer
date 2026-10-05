#include <cpr/cpr.h>
#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include <openssl/hmac.h>
#include <string>
#include <uuid.h>
#include <random>
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
	return signature;
}

std::string generateRequestId() {
    static std::random_device rd;
    static auto seed_data = std::array<int, std::mt19937::state_size>{};
    static bool seeded = [] {
        std::generate(seed_data.begin(), seed_data.end(), std::ref(rd));
        return true;
    }();
    static std::seed_seq seq(seed_data.begin(), seed_data.end());
    static std::mt19937 generator(seq);
    static uuids::uuid_random_generator gen(generator);

    return uuids::to_string(gen());
}

std::string generateUserStreamRequest() {
	std::ostringstream request;
	request << "{"
	    << "\"id\":\"" << generateRequestId() << "\","
	    << "\"method\":\"userDataStream.subscribe\","
	    << "\"params\":{}"
	    << "}";
	return request.str();
}
