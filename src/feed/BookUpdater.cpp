#include "BookUpdater.hpp"
#include "../book/OrderBookBuilder.hpp"
#include <iostream>
#include <sys/types.h>

uint64_t bookUpdater(SPSCQueue<RawMessage, CAPACITY> &queue, OrderBook &ob, std::atomic<bool> &running, uint64_t last_update_id) {
	uint64_t consumed = 0;
	while (running) {
		auto raw = queue.try_pop();
		if (!raw) continue;  // queue is empty
		++consumed;
		rapidjson::Document document;
		document.Parse(raw->data, raw->length);
		if (document.HasParseError()) {
			std::cout << "parse error\n";
			continue;
		}
		uint64_t U = document["U"].GetUint64();
		uint64_t u = document["u"].GetUint64();
		if (u < last_update_id + 1) continue;  // whole event is outdated
		if (U > last_update_id + 1) {
			std::cout << "GAP DETECTED: expected U = " << last_update_id + 1 << ", got U = " << U << '\n';
			// rebuild book - out of sync
			ob.clear();
			last_update_id = build_initial_orderbook(ob, 100);
			continue;
		}
		add_to_orderbook(ob, document["b"], BUY);
		add_to_orderbook(ob, document["a"], SELL);
		last_update_id = u;
		// notify strategy thread of book update
		ob.notify_update();
	}
	return consumed;
}

void add_to_orderbook(OrderBook& ob, const rapidjson::Value& orders, Side side) {
	for (const auto& order : orders.GetArray()) {
		ob.apply_delta(std::stod(order[0].GetString()), std::stod(order[1].GetString()), side);
	}
}
