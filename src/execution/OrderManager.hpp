#include "../types/OrderIntent.hpp"
#include "../types/SPSCQueue.hpp"

std::string sideToString(Side side);
void orderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderIntent, CAPACITY>& sendQueue, SPSCQueue<OrderIntent, CAPACITY>& updateQueue, std::atomic<bool>& running);
