#include "OrderBookBuilder.hpp"
#include <cpr/cpr.h>
#include <rapidjson/document.h>

void add_to_orderbook(OrderBook& ob, const auto& orders, Side side) {
	for (const auto& order : orders.GetArray()) {
		ob.add_order(std::stod(order[0].GetString()), std::stod(order[1].GetString()), side);
	}
}

OrderBook build_initial_orderbook() {
	OrderBook ob;
	cpr::Response r = cpr::Get(
		cpr::Url{"https://testnet.binance.vision/api/v3/depth"},
		cpr::Parameters{
			{"symbol", "BTCUSDT"},
			{"limit", "100"}
		}
	);
	rapidjson::Document document;
	document.Parse(r.text.c_str());
	add_to_orderbook(ob, document["bids"], BUY);
	add_to_orderbook(ob, document["asks"], SELL);
	return ob;
}
