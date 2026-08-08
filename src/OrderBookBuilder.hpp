#pragma once

#include "OrderBook.hpp"

OrderBook build_initial_orderbook(const std::string& symbol, size_t depth = 1000);
