#pragma once
#include "../types/OrderIntent.hpp"
#include "../types/SPSCQueue.hpp"

void executionLoop(SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool>& running);
