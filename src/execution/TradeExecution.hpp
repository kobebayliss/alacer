#include "../types/OrderIntent.hpp"
#include "../types/SPSCQueue.hpp"
#include "../types/OrderStatus.hpp"

void executionLoop(SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool>& running);
std::string sideToString(Side side);
