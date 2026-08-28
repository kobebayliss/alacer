#include "BacktestEventHandler.hpp"
#include <iostream>

void BacktestEventHandler::handleMessage(rapidjson::Document& document, SPSCQueue<RawMessage, CAPACITY>& queue) {
	RawMessage raw{};
	raw.length = std::min(static_cast<size_t>(document.GetStringLength()), sizeof(raw.data));
	memcpy(raw.data, document.GetString(), raw.length);
	if (!queue.try_push(raw)) {
		std::cout << "QUEUE FULL: DROPPING MESSAGE.\n";
	}
}
