#include "Strategy.hpp"
#include <atomic>
#include <iostream>
#include "../types/OrderIntent.hpp"

void strategyLoop(OrderBook &ob, SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool> &running, std::atomic<OrderStatus>& orderStatus) {
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
		OrderStatus expected;
		if (imbalance > 0.5) {
			expected = OrderStatus::NOT_HOLDING;
			if (orderStatus.compare_exchange_strong(expected, OrderStatus::PLACED_BUY, std::memory_order_acq_rel, std::memory_order_acquire)) {
				OrderIntent orderDetails{IntentType::PLACE, Side::BUY, top_k_bids.first[0].first, 0.01, 1};
				if (!orderQueue.try_push(orderDetails)) {
					std::cout << "QUEUE FULL: DROPPING ORDER.\n";
					orderStatus.store(OrderStatus::NOT_HOLDING, std::memory_order_release);
				}
			}
		} else if (imbalance < -0.5) {
			expected = OrderStatus::HOLDING;
			if (orderStatus.compare_exchange_strong(expected, OrderStatus::PLACED_SELL, std::memory_order_acq_rel, std::memory_order_acquire)) {
				OrderIntent orderDetails{IntentType::PLACE, Side::SELL, top_k_asks.first[0].first, 0.01, 1};
				if (!orderQueue.try_push(orderDetails)) {
					std::cout << "QUEUE FULL: DROPPING ORDER.\n";
					orderStatus.store(OrderStatus::HOLDING, std::memory_order_release);
				}
			}
		}
	}
}
