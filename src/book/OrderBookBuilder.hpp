#pragma once
#include <string>
#include "OrderBook.hpp"

void build_initial_orderbook(OrderBook& ob, const std::string& symbol, size_t depth = 1000);
