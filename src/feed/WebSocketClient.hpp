#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include <string>
#include "EventHandler.hpp"
#include <fstream>

class WebSocketClient {
	ix::WebSocket webSocket;
	std::ofstream outputFile;
	void setOnMessage(SPSCQueue<RawMessage, CAPACITY>& queue);
	void start();
	void stop();
public:
	explicit WebSocketClient(const std::string& url);
	~WebSocketClient();
	void openConnection(SPSCQueue<RawMessage, CAPACITY>& queue);
	void closeConnection();
};
