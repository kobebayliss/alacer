#include "Strategy.hpp"
#include <atomic>
#include <iostream>
#include <fstream>
#include "../types/OrderIntent.hpp"

void strategyLoop(OrderBook &ob, SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool> &running) {
	bool holding = false;
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
		if (imbalance > 0.5 && !holding) {
			OrderIntent orderDetails{IntentType::PLACE, Side::BUY, top_k_bids.first[0].first, 1, 1};
			if (!orderQueue.try_push(orderDetails)) {
				std::cout << "QUEUE FULL: DROPPING ORDER.\n";
			}
			holding = true;
		} else if (imbalance < -0.5 && holding) {
			holding = false;
		}
	}
}
