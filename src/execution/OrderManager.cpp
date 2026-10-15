#include <cpr/cpr.h>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include "OrderManager.hpp"
#include "../feed/BinanceAuth.hpp"

namespace {
	struct Position {
		double quantityHeld = 0;
		double workingBuyQty = 0;
		double workingSellQty = 0;
		double avgEntryPrice = 0;
		double realizedPnl = 0;
	};
	Position position;
	std::unordered_map<std::string, Order> orders;
}

static void applyFillToPos(Side side, double delta, double price) {
	if (side == Side::BUY) {
		position.workingBuyQty -= delta;
		const double newQty = position.quantityHeld + delta;
		position.avgEntryPrice = (position.quantityHeld * position.avgEntryPrice + delta * price) / newQty;
		position.quantityHeld = newQty;
	} else {
		position.workingSellQty -= delta;
		double closing;
		if (delta > position.quantityHeld) {
			closing = position.quantityHeld;
		} else {
			closing = delta;
		}
		position.realizedPnl += closing * (price - position.avgEntryPrice);
		position.quantityHeld -= closing;
		if (position.quantityHeld < EPSILON) {
			position.quantityHeld = 0;
			position.avgEntryPrice = 0;
		}
	}
}

static void applyUpdate(const OrderUpdate& update) {
	auto it = orders.find(update.clientOrderId);
	if (it == orders.end()) return;  // unknown id
	Order& order = it->second;
	OrderState& currentState = order.state;
	if (currentState == OrderState::FILLED || currentState == OrderState::CANCELED || currentState == OrderState::REJECTED || currentState == OrderState::EXPIRED) return;
	if (update.newState > order.state) order.state = update.newState;  // can do this since enum is ordered intentionally forward
	double delta = update.totalFilledVolume - order.filledVolume;
	if (delta > 0) {
		order.filledVolume = update.totalFilledVolume;
		applyFillToPos(order.side, delta, update.filledPrice);
	}
	if (order.state >= OrderState::FILLED) orders.erase(it);
	std::cout << "POS qty=" << position.quantityHeld << " avg=" << position.avgEntryPrice
		  << " realized=" << position.realizedPnl << '\n';
}

void orderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderRequest, CAPACITY>& sendQueue, SPSCQueue<OrderUpdate, CAPACITY>& updateQueue, std::atomic<bool>& running) {
	while (running) {
		while (auto update = updateQueue.try_pop()) {
			applyUpdate(*update);
		}
		// check fills/rejections from binance
		// checks for acknowledgements outgoing orders were success
		// place new orders from intent queue
		auto pIntent = intentQueue.try_pop();
		if (!pIntent) {
			continue; // empty
		}
		// decide if we still want to execute order
		// hands off order details to network sender thread to not block order manager
		// construct actual uuid first
		OrderRequest request{};
		std::string id = generateRequestId();
		uint64_t timestamp = getTimestampMillis();
		OrderIntent intent = *pIntent;
		orders.try_emplace(id, intent.side, OrderState::PENDING_NEW, intent.price, intent.volume, 0, timestamp, timestamp);
		if (intent.side == Side::BUY) {
			position.workingBuyQty += intent.volume;
		} else {
			position.workingSellQty += intent.volume;
		}
		memcpy(request.clientOrderId, id.c_str(), id.size() + 1);
		request.intent = intent;
		if (!sendQueue.try_push(request)) {
			std::cout << "send queue is full.\n";
		}
	}
}
