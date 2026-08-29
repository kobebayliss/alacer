#pragma once
#include "../book/OrderBook.hpp"

enum class IntentType { PLACE, CANCEL };

struct OrderIntent {
	IntentType type;
	Side side;
	double price;
	double volume;
	uint64_t client_order_id;
};
