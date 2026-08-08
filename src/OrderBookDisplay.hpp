#pragma once

#include "OrderBook.hpp"

namespace OrderBookDisplay {
	void print(const OrderBook& ob, size_t depth = 20);
	void printTopOfBook(const OrderBook& ob);
}
