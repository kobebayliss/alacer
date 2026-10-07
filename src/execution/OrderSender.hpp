#pragma once
#include "../types/SPSCQueue.hpp"
#include "../types/OrderIntent.hpp"
#include <atomic>

void orderSender(SPSCQueue<OrderIntent, CAPACITY>& sendQueue, std::atomic<bool>& running);
