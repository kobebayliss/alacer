#include "../types/OrderIntent.hpp"
#include "../types/SPSCQueue.hpp"

std::string sideToString(Side side);
void executionLoop(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderIntent, CAPACITY>& updateQueue, std::atomic<bool>& running);
void placeOrder();
