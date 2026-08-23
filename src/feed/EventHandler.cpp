#include "EventHandler.hpp"
#include <memory.h>
#include <iostream>
#include <fstream>

namespace EventHandler {
	void handleMessage(const ix::WebSocketMessagePtr &msg, SPSCQueue<RawMessage, CAPACITY>& queue, std::ofstream& outputFile) {
		if (msg->type == ix::WebSocketMessageType::Message)
		{
			outputFile << msg->str.data() << '\n';
			RawMessage raw{};
			raw.length = std::min(msg->str.size(), sizeof(raw.data));
			memcpy(raw.data, msg->str.data(), raw.length);
			if (!queue.try_push(raw)) {
				std::cout << "QUEUE FULL: DROPPING MESSAGE.\n";
			}
		}
		else if (msg->type == ix::WebSocketMessageType::Open)
		{
			std::cout << "Connection established\n";
		}
		else if (msg->type == ix::WebSocketMessageType::Error)
		{
			std::cout << "Connection error: " << msg->errorInfo.reason << '\n';
		}
	}
}
