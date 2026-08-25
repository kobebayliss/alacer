#include "WebSocketClient.hpp"
#include <atomic>
#include <iostream>

void WebSocketClient::setOnMessage(SPSCQueue<RawMessage, CAPACITY>& queue) {
	webSocket.setOnMessageCallback([this, &queue](const ix::WebSocketMessagePtr& msg) {
		EventHandler::handleMessage(msg, queue, outputFile, connected, produced);
	});
}
WebSocketClient::WebSocketClient(const std::string& url) : outputFile("data/updates5.json"), connected(false), produced(0) {
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
	webSocket.start();
}
void WebSocketClient::closeConnection() {
	connected.store(false, std::memory_order_release);
	webSocket.stop();
	outputFile.close();
}
bool WebSocketClient::isConnected() const {
	return connected.load(std::memory_order_acquire);
}
