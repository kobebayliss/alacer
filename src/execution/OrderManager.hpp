#include "../types/OrderTypes.hpp"
#include "../types/SPSCQueue.hpp"

void orderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderRequest, CAPACITY>& sendQueue, SPSCQueue<OrderUpdate, CAPACITY>& updateQueue, std::atomic<PositionView>& positionView, double startingMidPrice, std::atomic<bool>& running);
