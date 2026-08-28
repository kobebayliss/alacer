#pragma once
#include "OrderBook.hpp"
#include <cstdint>

uint64_t build_initial_orderbook(OrderBook& ob, std::string& data);
uint64_t build_initial_orderbook(OrderBook& ob);
