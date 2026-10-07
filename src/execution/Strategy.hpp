#pragma once
#include "../book/OrderBook.hpp"
#include "../types/OrderTypes.hpp"
#include "../types/SPSCQueue.hpp"

void strategyLoop(OrderBook& ob, SPSCQueue<OrderIntent, CAPACITY>& intentQueue, std::atomic<bool>& running);
