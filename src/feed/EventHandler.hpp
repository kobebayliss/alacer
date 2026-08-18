#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include "SPSCQueue.hpp"

const uint64_t CAPACITY = 4096;

struct RawMessage {
	char data[16384];
	uint64_t length;
};

namespace EventHandler {
	void handleMessage(const ix::WebSocketMessagePtr& msg, SPSCQueue<RawMessage, CAPACITY>& queue);
}
