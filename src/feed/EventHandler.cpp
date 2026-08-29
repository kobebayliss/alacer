#include "EventHandler.hpp"
#include <atomic>
#include <memory.h>
#include <iostream>

namespace EventHandler {
	void handleMessage(const ix::WebSocketMessagePtr &msg, SPSCQueue<RawMessage, CAPACITY>& queue, std::atomic<bool>& connected, uint64_t& produced) {
		if (msg->type == ix::WebSocketMessageType::Message)
		{
			RawMessage raw{};
			raw.length = std::min(msg->str.size(), sizeof(raw.data));
			memcpy(raw.data, msg->str.data(), raw.length);
			if (!queue.try_push(raw)) {
				std::cout << "QUEUE FULL: DROPPING MESSAGE.\n";
			}
			++produced;
		}
		else if (msg->type == ix::WebSocketMessageType::Open)
		{
			std::cout << "Connection established\n";
			connected.store(true, std::memory_order_release);
		}
		else if (msg->type == ix::WebSocketMessageType::Error)
		{
			std::cout << "Connection error: " << msg->errorInfo.reason << '\n';
		}
	}
}
