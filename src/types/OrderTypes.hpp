#pragma once
#include <cstdint>
#include <functional>
#include "Side.hpp"

enum class IntentType : uint8_t { PLACE, CANCEL };
enum class OrderState : uint8_t { PENDING_NEW, NEW, PARTIALLY_FILLED, FILLED, REJECTED, CANCELED, EXPIRED };
constexpr uint32_t CLIENT_ORDER_ID_LENGTH = 37; // 36 chars for UUID + 1 for null terminator

struct Order {
	Side side;
	OrderState state = OrderState::PENDING_NEW;
	double price;
	double volume;
	double filledVolume = 0;
	uint64_t createdTimestamp;
	uint64_t lastUpdateTimestamp;
};

struct OrderIntent {
	double price;
	double volume;
	IntentType type;
	Side side;
};

struct OrderRequest {
	OrderIntent intent;
	char clientOrderId[CLIENT_ORDER_ID_LENGTH];
};

struct OrderUpdate {
	OrderState newState;
	double filledPrice;
	double totalFilledVolume;
	uint64_t timestamp;
	char clientOrderId[CLIENT_ORDER_ID_LENGTH];
};
