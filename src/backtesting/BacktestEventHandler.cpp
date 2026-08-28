#include "BacktestEventHandler.hpp"
#include <iostream>

void BacktestEventHandler::handleMessage(const char* data, size_t length, SPSCQueue<RawMessage, CAPACITY>& queue) {
	RawMessage raw{};
	raw.length = std::min(length, sizeof(raw.data));
	memcpy(raw.data, data, raw.length);
	queue.try_push(raw);
	// if (!queue.try_push(raw)) {
	// 	std::cout << "QUEUE FULL: DROPPING MESSAGE.\n";
	// }
}
