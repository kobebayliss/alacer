#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include "../types/SPSCQueue.hpp"
#include "../types/RawMessage.hpp"

namespace EventHandler {
	void handleMessage(const ix::WebSocketMessagePtr& msg, SPSCQueue<RawMessage, CAPACITY>& eventQueue, std::atomic<bool>& connected, uint64_t& produced);
}
