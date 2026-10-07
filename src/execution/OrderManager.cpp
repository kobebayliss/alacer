#include <cpr/cpr.h>
#include <iostream>
#include <string>
#include "OrderManager.hpp"

// replace OrderIntent with shape for updates from binance
void orderManager(SPSCQueue<OrderIntent, CAPACITY>& intentQueue, SPSCQueue<OrderRequest, CAPACITY>& sendQueue, SPSCQueue<OrderIntent, CAPACITY>& updateQueue, std::atomic<bool>& running) {
	while (running) {
		// check fills/rejections from binance
		// checks for acknowledgements outgoing orders were success
		// place new orders from intent queue
		auto intent = intentQueue.try_pop();
		if (!intent) {
			continue; // empty
		}
		// decide if we still want to execute order
		// hands off order details to network sender thread to not block order manager
		// construct actual uuid first
		OrderRequest orderDetails{"1", *intent};
		if (!sendQueue.try_push(orderDetails)) {
			std::cout << "send queue is full.\n";
		}
	}
}
