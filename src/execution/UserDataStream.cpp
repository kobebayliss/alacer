#include <cpr/cpr.h>
#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include "UserDataStream.hpp"
#include "TradeExecution.hpp"
#include "../feed/WebSocketClient.hpp"

void userDataStream() {
	auto keys = getBinanceKeys();
	const std::string apiKey = keys[0];
	const std::string secretKey = keys[1];

	ix::WebSocket webSocket;
	webSocket.setUrl("wss://ws-api.testnet.binance.vision:443/ws-api/v3");
	webSocket.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg)
	{
		if (msg->type == ix::WebSocketMessageType::Message)
		{
			std::cout << "received message: " << msg->str << std::endl;
			std::cout << "> " << std::flush;
		}
		else if (msg->type == ix::WebSocketMessageType::Open)
		{
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
			webSocket.send(request.str());
			std::cout << "> " << std::flush;
		}
		else if (msg->type == ix::WebSocketMessageType::Error)
		{
			// Maybe SSL is not configured properly
			std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
			std::cout << "> " << std::flush;
		}
	});

	webSocket.start();
}
