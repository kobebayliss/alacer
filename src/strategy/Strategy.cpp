#include "Strategy.hpp"
#include "OrderIntent.hpp"
#include <atomic>
#include <iostream>

void strategyLoop(OrderBook &ob, std::atomic<bool> &running) {
	double running_price;
	bool holding = false;
	double holding_price;
	double profit = 0.0;
	while (running) {
		ob.updated.wait(false, std::memory_order_acquire);
		ob.updated.store(false, std::memory_order_relaxed);
		if (!running) [[unlikely]] break;

		const size_t k = 5;
		auto top_k_bids = ob.get_top_k_levels(k, BUY);
		auto top_k_asks = ob.get_top_k_levels(k, SELL);
		running_price = (top_k_bids.first[0].first + top_k_asks.first[0].first) / 2.0;
		double bid_volume = top_k_bids.second;
		double ask_volume = top_k_asks.second;
		double imbalance = (bid_volume - ask_volume) / (bid_volume + ask_volume);
		if (imbalance > 0.9 && !holding) {
			holding_price = running_price;
			holding = true;
		} else if (imbalance < -0.9 && holding) {
			std::cout << "BOUGHT AT: $" << holding_price << " | SOLD AT: $" << running_price;
			profit += (running_price - holding_price);
			holding = false;
		}
		std::cout << "IMBALANCE IS: " << imbalance << '\n';
	}
	std::cout << "PROFIT: " << profit;
}
