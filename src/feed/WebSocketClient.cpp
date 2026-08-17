#include "WebSocketClient.hpp"

void WebSocketClient::setOnMessage(SPSCQueue<RawMessage, CAPACITY>& queue) {
	webSocket.setOnMessageCallback([&queue](const ix::WebSocketMessagePtr& msg) {
		EventHandler::handleMessage(msg, queue);
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
void WebSocketClient::openConnection(SPSCQueue<RawMessage, CAPACITY>& queue) {
	this->setOnMessage(queue);
	this->start();
}
void WebSocketClient::closeConnection() {
	this->stop();
}
