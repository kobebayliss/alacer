#pragma once
#include "../types/SPSCQueue.hpp"
#include "../types/OrderTypes.hpp"
#include <atomic>

std::string sideToString(Side side);
void orderSender(SPSCQueue<OrderRequest, CAPACITY>& sendQueue, std::atomic<bool>& running);
