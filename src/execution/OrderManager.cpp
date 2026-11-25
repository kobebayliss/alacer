#include <atomic>
#include <cpr/cpr.h>
#include <cstring>
#include <fstream>
#include <iomanip>
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
	std::ofstream positionLog;
}

static const char* stateToString(OrderState state) {
	switch (state) {
		case OrderState::PENDING_NEW: return "PENDING_NEW";
		case OrderState::NEW: return "NEW";
		case OrderState::PARTIALLY_FILLED: return "PARTIALLY_FILLED";
		case OrderState::FILLED: return "FILLED";
		case OrderState::REJECTED: return "REJECTED";
		case OrderState::CANCELED: return "CANCELED";
		case OrderState::EXPIRED: return "EXPIRED";
	}
	return "UNKNOWN";
}

// appends a snapshot of the position after an update to data/position.txt
static void logPosition(const OrderUpdate& update, const Order& order, double fillQty) {
	if (!positionLog.is_open()) return;
	positionLog << std::fixed << std::setprecision(8)
		<< "=== " << update.timestamp << " | " << update.clientOrderId << " ===\n"
		<< "order:          " << (order.side == Side::BUY ? "BUY" : "SELL") << ' ' << stateToString(order.state)
		<< " | filled " << order.filledVolume << " / " << order.volume << '\n'
		<< "fill:           " << (fillQty > 0 ? fillQty : 0.0) << " @ " << std::setprecision(2) << update.filledPrice << '\n'
		<< std::setprecision(8)
		<< "quantityHeld:   " << position.quantityHeld << '\n'
		<< "workingBuyQty:  " << position.workingBuyQty << '\n'
		<< "workingSellQty: " << position.workingSellQty << '\n'
		<< std::setprecision(2)
		<< "avgEntryPrice:  " << position.avgEntryPrice << '\n'
		<< "realizedPnl:    " << position.realizedPnl << "\n\n";
	positionLog.flush();
}

static void adjustWorkingQty(Side side, double qty) {
	if (side == Side::BUY) {
		position.workingBuyQty -= qty;
	} else {
		position.workingSellQty -= qty;
	}
}

static void applyFillToPos(Side side, double fillQty, double price) {
	adjustWorkingQty(side, fillQty);
	if (side == Side::BUY) {
		const double newQty = position.quantityHeld + fillQty;
		position.avgEntryPrice = (position.quantityHeld * position.avgEntryPrice + fillQty * price) / newQty;
		position.quantityHeld = newQty;
	} else {
		double closing;
		if (fillQty > position.quantityHeld) {
			closing = position.quantityHeld;
		} else {
			closing = fillQty;
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
	double fillQty = update.totalFilledVolume - order.filledVolume;
	if (fillQty > 0) {
		order.filledVolume = update.totalFilledVolume;
		applyFillToPos(order.side, fillQty, update.filledPrice);
	}
	if (order.state == OrderState::CANCELED || order.state == OrderState::EXPIRED || order.state == OrderState::REJECTED) {
		// need to adjust exposure due to failing order
		adjustWorkingQty(order.side, order.volume - order.filledVolume);
	}
	if (order.state >= OrderState::FILLED) orders.erase(it);
	logPosition(update, order, fillQty);
}

void orderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderRequest, CAPACITY>& sendQueue, SPSCQueue<OrderUpdate, CAPACITY>& updateQueue, std::atomic<PositionView>& positionView, std::atomic<bool>& running) {
	bool updated;
	PositionView initialPosition = positionView.load(std::memory_order_acquire);
	position.quantityHeld = initialPosition.quantityHeld;
	positionLog.open("data/position.txt");
	if (!positionLog.is_open()) std::cerr << "Failed to open data/position.txt\n";
	while (running) {
		updated = false;
		while (auto update = updateQueue.try_pop()) {
			applyUpdate(*update);
			updated = true;
		}
		if (updated) {
			positionView.store(PositionView{position.quantityHeld, position.workingBuyQty, position.workingSellQty}, std::memory_order_release);
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
		positionView.store(PositionView{position.quantityHeld, position.workingBuyQty, position.workingSellQty}, std::memory_order_release);
		memcpy(request.clientOrderId, id.c_str(), id.size() + 1);
		request.intent = intent;
		if (!sendQueue.try_push(request)) {
			std::cout << "send queue is full.\n";
		}
	}
}
