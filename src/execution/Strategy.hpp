#pragma once
#include "../book/OrderBook.hpp"
#include "../types/OrderIntent.hpp"
#include "../types/SPSCQueue.hpp"

void strategyLoop(OrderBook& ob, SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool>& running);
