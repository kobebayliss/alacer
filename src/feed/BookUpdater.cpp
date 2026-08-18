#include "BookUpdater.hpp"
#include "../book/OrderBookBuilder.hpp"
#include <iostream>
#include <sys/types.h>

void bookUpdater(SPSCQueue<RawMessage, CAPACITY> &queue, OrderBook &ob, std::atomic<bool> &running) {
	int64_t last_applied_update = -1;
	while (running) {
		auto raw = queue.try_pop();
		if (!raw) continue;  // queue is empty
		rapidjson::Document document;
		document.Parse(raw->data, raw->length);
		if (document.HasParseError()) {
			std::cout << "parse error\n";
			continue;
		}
		int64_t U = document["U"].GetInt64();
		int64_t u = document["u"].GetInt64();
		if (U != last_applied_update + 1 && last_applied_update != 0) {
			std::cout << "GAP DETECTED: expected U = " << last_applied_update + 1 << ", got U = " << U << '\n';
			// rebuild book - out of sync
			ob.clear();
			build_initial_orderbook(ob, 100);
		}
		add_to_orderbook(ob, document["b"], BUY);
		add_to_orderbook(ob, document["a"], SELL);
		last_applied_update = u;
	}
}

void add_to_orderbook(OrderBook& ob, const rapidjson::Value& orders, Side side) {
	for (const auto& order : orders.GetArray()) {
		ob.apply_delta(std::stod(order[0].GetString()), std::stod(order[1].GetString()), side);
	}
}
