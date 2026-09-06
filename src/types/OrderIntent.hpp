#pragma once
#include <functional>
#include "Side.hpp"

enum class IntentType { PLACE, CANCEL };

enum class orderStatus { NOT_HOLDING, PLACED_BUY, HOLDING, PLACED_SELL };

struct OrderIntent {
	IntentType type;
	Side side;
	double price;
	double volume;
	uint64_t client_order_id;
};
