#include <cpr/cpr.h>
#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include "UserDataStream.hpp"
#include "TradeExecution.hpp"
#include "../types/WebSocketClient.hpp"

std::string generateUserDataRequest() {
	auto keys = getBinanceKeys();
	const std::string apiKey = keys[0];
	const std::string secretKey = keys[1];

	std::cout << "Connection established" << std::endl;
	int64_t timestamp = getTimestampMillis();
	std::string queryString = "apiKey=" + apiKey + "&timestamp=" + std::to_string(timestamp);
	std::string signature = hmacSha256Hex(secretKey, queryString);
	std::ostringstream request;
	request << "{"
		<< "\"id\":\"userdatastream-sub-1\","
		<< "\"method\":\"userDataStream.subscribe.signature\","
		<< "\"params\":{"
		    << "\"apiKey\":\"" << apiKey << "\","
		    << "\"timestamp\":" << timestamp << ","
		    << "\"signature\":\"" << signature << "\""
		<< "}"
		<< "}";
	return request.str();
}
