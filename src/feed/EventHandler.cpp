#include "EventHandler.hpp"
#include <iostream>

namespace EventHandler {
	void handleMessage(const ix::WebSocketMessagePtr &msg, SPSCQueue<RawMessage, CAPACITY>& queue) {
		if (msg->type == ix::WebSocketMessageType::Message)
		{
			RawMessage raw{};
			raw.length = std::min(msg->str.size(), sizeof(raw.data));
			std::memcpy(raw.data, msg->str.data(), raw.length);
			if (!queue.try_push(raw)) {
				std::cout << "QUEUE FULL: DROPPING MESSAGE." << std::endl;
			} else {
				std::cout << "MESSAGED INSERTED TO QUEUE." << std::endl;
			}

		}
		else if (msg->type == ix::WebSocketMessageType::Open)
		{
			std::cout << "Connection established" << std::endl;
		}
		else if (msg->type == ix::WebSocketMessageType::Error)
		{
			std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
		}
	}
}
