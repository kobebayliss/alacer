#include "../types/OrderIntent.hpp"
#include "../types/SPSCQueue.hpp"

std::string sideToString(Side side);
void executionLoop(SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool>& running);
