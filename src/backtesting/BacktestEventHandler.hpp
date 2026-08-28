#pragma once
#include "../feed/SPSCQueue.hpp"
#include <rapidjson/document.h>

namespace BacktestEventHandler {
	void handleMessage(rapidjson::Document& document, SPSCQueue<RawMessage, CAPACITY>& queue);
}
