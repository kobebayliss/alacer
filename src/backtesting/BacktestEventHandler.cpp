#include <thread>
#include "BacktestEventHandler.hpp"

void BacktestEventHandler::handleMessage(const char* data, size_t length, SPSCQueue<RawMessage, CAPACITY>& eventQueue) {
	RawMessage raw{};
	raw.length = std::min(length, sizeof(raw.data));
	memcpy(raw.data, data, raw.length);
	while (!eventQueue.try_push(raw)) {
		std::this_thread::yield();
	}
}
