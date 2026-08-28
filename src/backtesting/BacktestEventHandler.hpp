#pragma once
#include "../feed/SPSCQueue.hpp"
#include <rapidjson/document.h>

namespace BacktestEventHandler {
	void handleMessage(const char* data, size_t length, SPSCQueue<RawMessage, CAPACITY>& queue);
}
