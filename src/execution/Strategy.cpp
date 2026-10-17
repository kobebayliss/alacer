#include "Strategy.hpp"
#include <algorithm>
#include <atomic>
#include <iostream>

constexpr double MAX_ORDER_QTY = 0.05;
constexpr double MAX_POSITION = 0.1;
constexpr double IMB_THRESHOLD = 0.6;
constexpr double MIN_DEPTH = 0.5;
constexpr double LIQ_FRACTION = 0.5;
constexpr double QTY_STEP = 0.00001;
constexpr double MIN_NOTIONAL = 5.0;


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
		if (imbalance > IMB_THRESHOLD) {
			double headroom = MAX_POSITION - (snapshot.quantityHeld + snapshot.workingBuyQty);
			if (headroom < QTY_STEP) continue;
			const double signalStrength = (imbalance - IMB_THRESHOLD) / (1.0 - IMB_THRESHOLD);
			const double bestAskQty = top_k_asks.first[0].second;
			const double price = top_k_asks.first[0].first;
			// determines qty to buy whilst ensuring we dont walk the book by comparing top ask qty
			double qty = std::min({MAX_ORDER_QTY, headroom * signalStrength, bestAskQty * LIQ_FRACTION});
			qty = std::floor(qty / QTY_STEP + EPSILON) * QTY_STEP;
			if (qty < QTY_STEP || qty * price < MIN_NOTIONAL) continue;
			OrderIntent orderDetails{price, qty, IntentType::PLACE, Side::BUY};
			if (!intentQueue.try_push(orderDetails)) {
				std::cout << "QUEUE FULL: DROPPING ORDER.\n";
			} 
		} else if (imbalance < -IMB_THRESHOLD) {
			double balance = snapshot.quantityHeld - snapshot.workingSellQty;
			if (balance < QTY_STEP) continue;
			const double signalStrength = (-imbalance - IMB_THRESHOLD) / (1.0 - IMB_THRESHOLD);
			const double bestBidQty = top_k_bids.first[0].second;
			const double price = top_k_bids.first[0].first;
			// determines qty to sell whilst ensuring we dont walk the book by comparing top bid qty
			double qty = std::min({balance * signalStrength, bestBidQty * LIQ_FRACTION});
			qty = std::floor(qty / QTY_STEP + EPSILON) * QTY_STEP;
			if (qty < QTY_STEP || qty * price < MIN_NOTIONAL) continue;
			OrderIntent orderDetails{price, qty, IntentType::PLACE, Side::SELL};
			if (!intentQueue.try_push(orderDetails)) {
				std::cout << "QUEUE FULL: DROPPING ORDER.\n";
			} 
		}
	}
}
