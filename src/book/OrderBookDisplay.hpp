#pragma once
#include "OrderBook.hpp"
#include <atomic>

namespace OrderBookDisplay {
	void printLoop(const OrderBook& ob, std::atomic<bool>& running);
	void print(const OrderBook& ob);
	void printTopOfBook(const OrderBook& ob);
}
