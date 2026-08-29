#pragma once
#include "../book/OrderBook.hpp"

void strategyLoop(OrderBook& ob, std::atomic<bool>& running);
