#include "../types/OrderTypes.hpp"
#include "../types/SPSCQueue.hpp"

void orderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderRequest, CAPACITY>& sendQueue, SPSCQueue<OrderIntent, CAPACITY>& updateQueue, std::atomic<bool>& running);
