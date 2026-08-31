#include <cpr/cpr.h>
#include <rapidjson/document.h>
#include <iostream>
#include <fstream>
#include "OrderBookBuilder.hpp"
#include "../feed/BookUpdater.hpp"

uint64_t build_initial_orderbook(OrderBook& ob, std::string& data) {
	if (data[0] == '\0') { // NOT backesting (else we pass the OB initial state)
		cpr::Response r = cpr::Get(
			cpr::Url{"https://testnet.binance.vision/api/v3/depth"},
			cpr::Parameters{
				{"symbol", ob.instrument},
				{"limit", std::to_string(DEPTH)}
			}
		);
		data = r.text;
	}
	rapidjson::Document document;
	document.Parse(data.c_str());

	if (document.HasParseError() || !document.HasMember("bids") || !document.HasMember("asks")) {
		std::cout << "FAILED TO BUILD ORDER BOOK: " << data << '\n';
		return 0;
	}
	add_to_orderbook(ob, document["bids"], Side::BUY);
	add_to_orderbook(ob, document["asks"], Side::SELL);
	return document["lastUpdateId"].GetUint64();
}

uint64_t build_initial_orderbook(OrderBook& ob) {
	std::string data;
	return build_initial_orderbook(ob, data);
}
