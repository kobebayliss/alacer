#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include <string>
#include "EventHandler.hpp"
#include <fstream>

class WebSocketClient {
	ix::WebSocket webSocket;
	std::atomic<bool> connected;
	std::atomic<bool> snapshotWritten;
	void setOnMessage(SPSCQueue<RawMessage, CAPACITY>& queue);
	void start();
	void stop();
public:
	explicit WebSocketClient(const std::string& url, const std::string& outputPath);
	~WebSocketClient();
	void openConnection(SPSCQueue<RawMessage, CAPACITY>& queue);
	void closeConnection();
	void writeSnapshot(const std::string& snapshotJson);
	bool isConnected() const;
	uint64_t produced;
	std::ofstream outputFile;
};
