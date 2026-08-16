#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include <string>

class WebSocketClient {
	ix::WebSocket webSocket;
public:
	explicit WebSocketClient(const std::string& url);
	~WebSocketClient();
	void setOnMessage(auto callback);
	void start();
	void stop();
};
