#pragma once
#include <cstdint>
#include <functional>
#include "Side.hpp"

enum class IntentType { PLACE, CANCEL };
constexpr uint32_t CLIENT_ORDER_ID_LENGTH = 37; // 36 chars for UUID + 1 for null terminator

struct OrderIntent {
	IntentType type;
	Side side;
	double price;
	double volume;
};

struct OrderRequest {
	char clientOrderId[CLIENT_ORDER_ID_LENGTH];
	OrderIntent intent;
};

struct OrderAck {
	std::string clientOrderId;
	bool success;
};
