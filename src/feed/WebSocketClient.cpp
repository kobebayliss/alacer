#include "WebSocketClient.hpp"
#include <iostream>

void WebSocketClient::setOnMessage() {
	webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg)
	{
		if (msg->type == ix::WebSocketMessageType::Message)
		{
			std::cout << "received message: " << msg->str << std::endl;
			std::cout << "> " << std::flush;
		}
		else if (msg->type == ix::WebSocketMessageType::Open)
		{
			std::cout << "Connection established" << std::endl;
			std::cout << "> " << std::flush;
		}
		else if (msg->type == ix::WebSocketMessageType::Error)
		{
			std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
			std::cout << "> " << std::flush;
		}
	});
}
void WebSocketClient::start() {
	webSocket.start();
}
void WebSocketClient::stop() {
	webSocket.stop();
}
WebSocketClient::WebSocketClient(const std::string& url) {
	webSocket.setUrl(url);
}
WebSocketClient::~WebSocketClient() {
	webSocket.stop();
}
void WebSocketClient::openConnection() {
	this->setOnMessage();
	this->start();
}
void WebSocketClient::closeConnection() {
	this->stop();
}
