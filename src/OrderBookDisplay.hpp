#pragma once

#include "OrderBook.hpp"

namespace OrderBookDisplay {
	void print(const OrderBook& ob, size_t depth = 15);
	void printTopOfBook(const OrderBook& ob);
}
