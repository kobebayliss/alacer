#include "WebSocketClient.hpp"
#include <iostream>

void WebSocketClient::setOnMessage(SPSCQueue<RawMessage, CAPACITY>& queue) {
	webSocket.setOnMessageCallback([this, &queue](const ix::WebSocketMessagePtr& msg) {
		EventHandler::handleMessage(msg, queue, outputFile);
	});
}
void WebSocketClient::start() {
	webSocket.start();
}
void WebSocketClient::stop() {
	webSocket.stop();
}
WebSocketClient::WebSocketClient(const std::string& url) : outputFile("data/updates.json") {
	webSocket.setUrl(url);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Could not open the file!" << std::endl;
	}
}
WebSocketClient::~WebSocketClient() {
	webSocket.stop();
}
void WebSocketClient::openConnection(SPSCQueue<RawMessage, CAPACITY>& queue) {
	this->setOnMessage(queue);
	this->start();
}
void WebSocketClient::closeConnection() {
	this->stop();
	outputFile.close();
}
