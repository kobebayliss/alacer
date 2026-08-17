#include "BookUpdater.hpp"
#include <iostream>

void bookUpdater(SPSCQueue<RawMessage, CAPACITY> &queue, OrderBook &ob, std::atomic<bool> &running) {
	while (running) {
		auto raw = queue.try_pop();
		if (!raw) continue;  // queue is empty
		rapidjson::Document document;
		document.Parse(raw->data, raw->length);
		if (document.HasParseError()) {
			std::cout << "parse error\n";
			continue;
		}
		add_to_orderbook(ob, document["b"], BUY);
		add_to_orderbook(ob, document["a"], SELL);
	}
}

void add_to_orderbook(OrderBook& ob, const rapidjson::Value& orders, Side side) {
	for (const auto& order : orders.GetArray()) {
		ob.apply_delta(std::stod(order[0].GetString()), std::stod(order[1].GetString()), side);
	}
}
