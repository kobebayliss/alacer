#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include <string>

class WebSocketClient {
	ix::WebSocket webSocket;
	void setOnMessage();
	void start();
	void stop();
public:
	explicit WebSocketClient(const std::string& url);
	~WebSocketClient();
	void openConnection();
	void closeConnection();
};
