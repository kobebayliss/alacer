#include <atomic>
#include <iostream>
#include "EventHandler.hpp"
#include "WebSocketClient.hpp"

void WebSocketClient::setOnMessage(SPSCQueue<RawMessage, CAPACITY>& eventQueue) {
	webSocket.setOnMessageCallback([this, &eventQueue](const ix::WebSocketMessagePtr& msg) {
		EventHandler::handleMessage(msg, eventQueue, connected, produced);
	});
}
WebSocketClient::WebSocketClient(const std::string& url, const std::string& outputPath) : connected(false), snapshotWritten(false), produced(0), outputFile(outputPath) {
	webSocket.setUrl(url);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Could not open the file!" << std::endl;
	}
}
WebSocketClient::~WebSocketClient() {
	webSocket.stop();
}
void WebSocketClient::openConnection(SPSCQueue<RawMessage, CAPACITY>& eventQueue) {
	this->setOnMessage(eventQueue);
	webSocket.start();
}
void WebSocketClient::closeConnection() {
	connected.store(false, std::memory_order_release);
	webSocket.stop();
	outputFile.close();
}
void WebSocketClient::writeSnapshot(const std::string& snapshotJson) {
	outputFile << snapshotJson << '\n';
	outputFile.flush();
	snapshotWritten.store(true, std::memory_order_release);
}
bool WebSocketClient::isConnected() const {
	return connected.load(std::memory_order_acquire);
}
