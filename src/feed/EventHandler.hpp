#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include "SPSCQueue.hpp"

namespace EventHandler {
	void handleMessage(const ix::WebSocketMessagePtr& msg, SPSCQueue<RawMessage, CAPACITY>& queue, std::atomic<bool>& connected, uint64_t& produced);
}
