#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include <string>
#include <fstream>
#include <iostream>

using MessageHandler = std::function<void(const ix::WebSocketMessagePtr&)>;

class WebSocketClient {
	ix::WebSocket webSocket;
	std::atomic<bool> connected;
	std::atomic<bool> snapshotWritten;
	MessageHandler handler;
public:
	explicit WebSocketClient(const std::string& url, const std::string& outputPath, MessageHandler handler) : connected(false), snapshotWritten(false), produced(0), handler(std::move(handler)), outputFile(outputPath) {
		webSocket.setUrl(url);
		if (!outputFile.is_open()) {
			std::cerr << "Error: Could not open the file!" << std::endl;
		}
	}
	~WebSocketClient() {
		webSocket.stop();
	}
	void openConnection() {
		webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
			if (msg->type == ix::WebSocketMessageType::Open) {
				connected.store(true, std::memory_order_release);
			} else if (msg->type == ix::WebSocketMessageType::Error) {
				connected.store(false, std::memory_order_release);
			}
			handler(msg);
			++produced;
		});
		webSocket.start();
	}
	void closeConnection() {
		webSocket.stop();
		connected.store(false, std::memory_order_release);
		outputFile.close();
	}
	void writeSnapshot(const std::string& snapshotJson) {
		outputFile << snapshotJson << '\n';
		outputFile.flush();
		snapshotWritten.store(true, std::memory_order_release);
	}
	bool isConnected() const {
		return connected.load(std::memory_order_acquire);
	}
	uint64_t produced;
	std::ofstream outputFile;
};
