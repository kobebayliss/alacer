#include "../types/OrderIntent.hpp"
#include "../types/SPSCQueue.hpp"
#include "../types/OrderStatus.hpp"

void executionLoop(SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool>& running, std::atomic<OrderStatus>& orderStatus);
std::string sideToString(Side side);
