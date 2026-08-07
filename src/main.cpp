#include <cpr/parameters.h>
#include <iomanip>
#include <rapidjson/document.h>
#include <iostream>
#include <cpr/cpr.h>
#include "OrderBook.hpp"

int main() {
	cpr::Response r = cpr::Get(
		cpr::Url{"https://testnet.binance.vision/api/v3/depth"},
		cpr::Parameters{
			{"symbol", "BTCUSDT"},
			{"limit", "100"}
		}
	);
	rapidjson::Document document;
	document.Parse(r.text.c_str());
	const auto& bids = document["bids"];
	OrderBook ob;
	for (const auto& bid : bids.GetArray()) {
		std::cout << "Price: " << bid[0].GetString() << std::endl << "Quantity: " << bid[1].GetString() << std::endl;
		ob.add_order(std::stod(bid[0].GetString()), std::stod(bid[1].GetString()), BUY);
	}
	const size_t x = 16;
	std::vector<double> top_x = ob.get_top_k_prices(x, BUY);
	for (size_t i{0}; i < x; i++) {
		std::cout << std::fixed << std::setprecision(6) << "top x orders: " << top_x[i] << std::endl;
	}
	return 0;
}
