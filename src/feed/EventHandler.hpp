#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include "../types/SPSCQueue.hpp"
#include "../types/RawMessage.hpp"
#include "../types/OrderStatus.hpp"

namespace EventHandler {
	void handleDepthUpdate(const ix::WebSocketMessagePtr& msg, SPSCQueue<RawMessage, CAPACITY>& eventQueue);
	void handleUserDataUpdate(const ix::WebSocketMessagePtr& msg);
}
