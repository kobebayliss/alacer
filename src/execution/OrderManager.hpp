#include "../types/OrderTypes.hpp"
#include "../types/SPSCQueue.hpp"

struct PositionView {
	bool hasOpenOrder;
	bool isLong;
};

void orderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderRequest, CAPACITY>& sendQueue, SPSCQueue<OrderUpdate, CAPACITY>& updateQueue, std::atomic<bool>& running);
