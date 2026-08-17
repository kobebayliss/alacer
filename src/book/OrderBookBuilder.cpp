#include "OrderBookBuilder.hpp"
#include <cpr/cpr.h>
#include <rapidjson/document.h>
#include "../feed/BookUpdater.hpp"

void build_initial_orderbook(OrderBook& ob, const std::string& symbol, size_t depth) {
	cpr::Response r = cpr::Get(
		cpr::Url{"https://testnet.binance.vision/api/v3/depth"},
		cpr::Parameters{
			{"symbol", symbol},
			{"limit", std::to_string(depth)}
		}
	);
	rapidjson::Document document;
	document.Parse(r.text.c_str());
	add_to_orderbook(ob, document["bids"], BUY);
	add_to_orderbook(ob, document["asks"], SELL);
}
