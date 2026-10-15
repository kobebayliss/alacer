#include "../types/OrderTypes.hpp"
#include "../types/SPSCQueue.hpp"

constexpr double MAX_POSITION = 0.1;
constexpr double EPSILON = 1e-9;

struct PositionView {
	bool hasOpenOrder;
	bool isLong;
};

void orderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderRequest, CAPACITY>& sendQueue, SPSCQueue<OrderUpdate, CAPACITY>& updateQueue, std::atomic<bool>& running);
