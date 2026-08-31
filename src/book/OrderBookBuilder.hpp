#pragma once
#include <cstdint>
#include "OrderBook.hpp"

uint64_t build_initial_orderbook(OrderBook& ob, std::string& data);
uint64_t build_initial_orderbook(OrderBook& ob);
