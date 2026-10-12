#include "Strategy.hpp"
#include <atomic>
#include <chrono>
#include <iostream>

void strategyLoop(OrderBook &ob, SPSCQueue<OrderIntent, CAPACITY>& intentQueue, std::atomic<bool> &running) {
	const bool TEST_MODE = true;  // true: trade on a timer, ignore imbalance
	const auto TEST_INTERVAL = std::chrono::seconds(5);

	bool holding = false;
	auto lastTrade = std::chrono::steady_clock::now() - TEST_INTERVAL;
	while (running) {
		ob.updated.wait(false, std::memory_order_acquire);
		ob.updated.store(false, std::memory_order_relaxed);
		if (!running) [[unlikely]] break;

		const size_t k = 5;
		auto top_k_bids = ob.get_top_k_levels(k, Side::BUY);
		auto top_k_asks = ob.get_top_k_levels(k, Side::SELL);
		double bid_volume = top_k_bids.second;
		double ask_volume = top_k_asks.second;
		double imbalance = (bid_volume - ask_volume) / (bid_volume + ask_volume);

		auto now = std::chrono::steady_clock::now();
		bool testBuy  = TEST_MODE && !holding && now - lastTrade >= TEST_INTERVAL;
		bool testSell = TEST_MODE && holding  && now - lastTrade >= TEST_INTERVAL;

		if ((imbalance > 0.8 && !holding && !TEST_MODE) || testBuy) {
			OrderIntent orderDetails{top_k_bids.first[0].first, 0.01, IntentType::PLACE, Side::BUY};
			if (!intentQueue.try_push(orderDetails)) {
				std::cout << "QUEUE FULL: DROPPING ORDER.\n";
			} else {
				holding = true;
				lastTrade = now;
			}
		} else if ((imbalance < -0.8 && holding && !TEST_MODE) || testSell) {
			OrderIntent orderDetails{top_k_asks.first[0].first, 0.01, IntentType::PLACE, Side::SELL};
			if (!intentQueue.try_push(orderDetails)) {
				std::cout << "QUEUE FULL: DROPPING ORDER.\n";
			} else {
				holding = false;
				lastTrade = now;
			}
		}
	}
}
