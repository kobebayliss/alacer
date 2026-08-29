#include "BacktestEventHandler.hpp"
#include <thread>

void BacktestEventHandler::handleMessage(const char* data, size_t length, SPSCQueue<RawMessage, CAPACITY>& queue) {
	RawMessage raw{};
	raw.length = std::min(length, sizeof(raw.data));
	memcpy(raw.data, data, raw.length);
	while (!queue.try_push(raw)) {
		std::this_thread::yield();
	}
}
