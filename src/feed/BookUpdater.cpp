#include "BookUpdater.hpp"
#include <iostream>
#include <sys/types.h>
#include <fstream>

uint64_t bookUpdater(SPSCQueue<RawMessage, CAPACITY> &queue, OrderBook &ob, std::atomic<bool> &running, uint64_t last_update_id, std::ofstream* outputFile) {
	uint64_t consumed = 0;
	while (running) {
		auto raw = queue.try_pop();
		if (!raw) {  // queue is empty
			continue;
		}
		++consumed;
		if (outputFile) *outputFile << std::string_view(raw->data, raw->length) << '\n';

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
			std::cerr << "GAP DETECTED: expected U = " << last_update_id + 1 << ", got U = " << U << '\n';
			std::exit(1);
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
