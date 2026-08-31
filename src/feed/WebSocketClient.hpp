#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include <string>
#include <fstream>
#include "../types/SPSCQueue.hpp"
#include "../types/RawMessage.hpp"

class WebSocketClient {
	ix::WebSocket webSocket;
	std::atomic<bool> connected;
	std::atomic<bool> snapshotWritten;
	void setOnMessage(SPSCQueue<RawMessage, CAPACITY>& eventQueue);
	void start();
	void stop();
public:
	explicit WebSocketClient(const std::string& url, const std::string& outputPath);
	~WebSocketClient();
	void openConnection(SPSCQueue<RawMessage, CAPACITY>& eventQueue);
	void closeConnection();
	void writeSnapshot(const std::string& snapshotJson);
	bool isConnected() const;
	uint64_t produced;
	std::ofstream outputFile;
};
