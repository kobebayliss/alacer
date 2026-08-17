#include "EventHandler.hpp"
#include <iostream>
#include <memory.h>

namespace EventHandler {
	void handleMessage(const ix::WebSocketMessagePtr &msg, SPSCQueue<RawMessage, CAPACITY>& queue) {
		if (msg->type == ix::WebSocketMessageType::Message)
		{
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
