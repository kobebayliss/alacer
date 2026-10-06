#include <atomic>
#include <memory.h>
#include <iostream>
#include "EventHandler.hpp"

namespace EventHandler {
	void handleDepthUpdate(const ix::WebSocketMessagePtr& msg, SPSCQueue<RawMessage, CAPACITY>& eventQueue) {
		RawMessage raw{};
		raw.length = std::min(msg->str.size(), sizeof(raw.data));
		memcpy(raw.data, msg->str.data(), raw.length);
		if (!eventQueue.try_push(raw)) {
			std::cout << "QUEUE FULL: DROPPING MESSAGE.\n";
		}
	}
	// replace OrderIntent with shape for updates from binance
	void handleUserDataUpdate(const ix::WebSocketMessagePtr& msg, SPSCQueue<OrderIntent, CAPACITY>& updateQueue) { 
		// add to update stream queue (consumed by OMS thread)
		std::cout << msg->str.data();
	}
}
