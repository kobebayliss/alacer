#include "Strategy.hpp"
#include "OrderIntent.hpp"
#include <atomic>

void strategyLoop(OrderBook &ob, std::atomic<bool> &running) {
	while (running) {
		ob.updated.wait(false, std::memory_order_acquire);
		ob.updated.store(false, std::memory_order_relaxed);
		if (!running) [[unlikely]] break;

		// price, volume
		std::pair<double, double> bid = ob.get_top_level(BUY);
		std::pair<double, double> ask = ob.get_top_level(SELL);

	}
}
