#pragma once
#include <rapidjson/document.h>
#include "../types/SPSCQueue.hpp"
#include "../types/RawMessage.hpp"

namespace BacktestEventHandler {
	void handleMessage(const char* data, size_t length, SPSCQueue<RawMessage, CAPACITY>& eventQueue);
}
