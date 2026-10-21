#include "Strategy.hpp"
#include <algorithm>
#include <chrono>
#include <atomic>
#include <iostream>
#include <thread>

constexpr double MAX_POSITION = 0.4;
constexpr double IMB_THRESHOLD = 0.6;
constexpr double MIN_DEPTH = 0.5;
constexpr double LIQ_FRACTION = 0.5;
constexpr double QTY_STEP = 0.00001;
constexpr double MIN_NOTIONAL = 5.0;
constexpr double MIN_TRADE = 0.001;


void strategyLoop(OrderBook &ob, SPSCQueue<OrderIntent, CAPACITY>& intentQueue, std::atomic<PositionView>& positionView, std::atomic<bool> &running) {
	while (running) {
		ob.updated.wait(false, std::memory_order_acquire);
		ob.updated.store(false, std::memory_order_relaxed);
		if (!running) [[unlikely]] break;

		const size_t k = 5;
		auto top_k_bids = ob.get_top_k_levels(k, Side::BUY);
		auto top_k_asks = ob.get_top_k_levels(k, Side::SELL);
		if (top_k_bids.first.empty() || top_k_asks.first.empty()) continue;
		double bid_volume = top_k_bids.second;
		double ask_volume = top_k_asks.second;
		double depth = bid_volume + ask_volume;
		PositionView snapshot = positionView.load(std::memory_order_acquire);
		if (depth < MIN_DEPTH) continue;

		double imbalance = (bid_volume - ask_volume) / depth;
		double targetPosition;
		if (imbalance > IMB_THRESHOLD) {
			const double signalStrength = (imbalance - IMB_THRESHOLD) / (1.0 - IMB_THRESHOLD);
			targetPosition = MAX_POSITION * signalStrength;
		} else if (imbalance < -IMB_THRESHOLD) {
			targetPosition = 0;
		} else {
			continue;
		}

		const double exposure = snapshot.quantityHeld + snapshot.workingBuyQty;
		const double diff = targetPosition - exposure;
		if (diff > MIN_TRADE) {
			const double price = top_k_asks.first[0].first;
			// determines qty to buy whilst ensuring we dont walk the book by comparing top ask qty
			double qty = std::min({diff, top_k_asks.first[0].second * LIQ_FRACTION});
			qty = std::floor(qty / QTY_STEP + EPSILON) * QTY_STEP;
			if (qty < QTY_STEP || qty * price < MIN_NOTIONAL) continue;
			OrderIntent orderDetails{price, qty, IntentType::PLACE, Side::BUY};
			if (!intentQueue.try_push(orderDetails)) {
				std::cout << "QUEUE FULL: DROPPING ORDER.\n";
			} 
		} else if (diff < -MIN_TRADE) {
			const double sellable = snapshot.quantityHeld - snapshot.workingSellQty;
			const double price = top_k_bids.first[0].first;
			// determines qty to sell whilst ensuring we dont walk the book by comparing top bid qty
			double qty = std::min({-diff, sellable, top_k_bids.first[0].second * LIQ_FRACTION});
			qty = std::floor(qty / QTY_STEP + EPSILON) * QTY_STEP;
			if (qty < QTY_STEP || qty * price < MIN_NOTIONAL) continue;
			OrderIntent orderDetails{price, qty, IntentType::PLACE, Side::SELL};
			if (!intentQueue.try_push(orderDetails)) {
				std::cout << "QUEUE FULL: DROPPING ORDER.\n";
			} 
		}
		std::cout << "imb=" << imbalance << "\ndepth=" << depth << "\ntarget position=" << targetPosition << '\n';
		std::this_thread::sleep_for(std::chrono::milliseconds(250)); // for debugging
	}
}
