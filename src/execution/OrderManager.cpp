#include <cpr/cpr.h>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include "OrderManager.hpp"
#include "../feed/BinanceAuth.hpp"

namespace {
	struct Position {
		double volume = 0;
		double avgEntryPrice = 0;
		double pnl = 0;
	};
	Position position;
	std::unordered_map<const char*, Order> orders;
}

static void applyUpdate(const OrderUpdate& update) {
	auto it = orders.find(update.clientOrderId);
	if (it == orders.end()) return;  // unknown id
	Order& order = it->second;
	OrderState& currentState = order.state;
	if (currentState == OrderState::FILLED || currentState == OrderState::CANCELED || currentState == OrderState::REJECTED || currentState == OrderState::EXPIRED) return;
	order.state = update.newState;
	double delta = update.totalFilledVolume - order.filledVolume;
	if (delta > 0) {
		order.filledVolume = update.totalFilledVolume;
		// do position updates
		if (order.side == Side::BUY) {
			position.volume += delta;
		} else {
			position.volume -= delta;
		}
	}
}

void orderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderRequest, CAPACITY>& sendQueue, SPSCQueue<OrderUpdate, CAPACITY>& updateQueue, std::atomic<bool>& running) {
	while (running) {
		while (auto update = updateQueue.try_pop()) {
			applyUpdate(*update);
		}
		// check fills/rejections from binance
		// checks for acknowledgements outgoing orders were success
		// place new orders from intent queue
		auto intent = intentQueue.try_pop();
		if (!intent) {
			continue; // empty
		}
		// decide if we still want to execute order
		// hands off order details to network sender thread to not block order manager
		// construct actual uuid first
		OrderRequest request{};
		std::string id = generateRequestId();
		memcpy(request.clientOrderId, id.c_str(), id.size() + 1);
		request.intent = *intent;
		if (!sendQueue.try_push(request)) {
			std::cout << "send queue is full.\n";
		}
	}
}
