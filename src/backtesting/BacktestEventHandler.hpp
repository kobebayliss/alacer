#pragma once
#include "../feed/SPSCQueue.hpp"

namespace BacktestEventHandler {
	void handleMessage(SPSCQueue<RawMessage, CAPACITY>& queue, std::ofstream& outputFile, std::atomic<bool>& connected, uint64_t& produced);
}
