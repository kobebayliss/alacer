#include <format>
#include <stdexcept>
#include "OrderBook.hpp"

void OrderBook::apply_delta(double price, double volume, Side side) {
	prices_map& map = (side == BUY) ? buy_orders : sell_orders;
	auto it = map.find(price);
	if (volume == 0.0) {
		if (it != map.end()) {
			map.erase(it);
		}
		return;
	}
	if (it == map.end()) {
		// new price level
		map.try_emplace(price, price, volume);
	} else {
		// existing price level
		it->second.volume = volume;
	}
}

std::pair<double, double> OrderBook::get_top_level(Side side) const {
	if (side == BUY) {
		auto it = buy_orders.rbegin();
		return {it->first, it->second.volume};
	}
	auto it = sell_orders.begin();
	return {it->first, it->second.volume};
}

std::vector<std::pair<double, double>> OrderBook::get_top_k_levels(size_t k, Side side) const {
	const prices_map& map = (side == BUY) ? buy_orders : sell_orders;
	k = std::min(k, map.size());
	std::vector<std::pair<double, double>> result;
	result.reserve(k);
	if (side == BUY) {
		auto it = map.rbegin();
		while (k--) {
			result.emplace_back(it->first, it->second.volume);
			it++;
		}
	} else {
		auto it = map.begin();
		while (k--) {
			result.emplace_back(it->first, it->second.volume);
			it++;
		}
	}
	return result;
}

double OrderBook::get_volume_at_price(double price, Side side) const {
	const prices_map& map = (side == BUY) ? buy_orders : sell_orders;
	auto it = map.find(price);
	if (it == map.end()) {
		throw std::invalid_argument(std::format("No orders at price {}", price));
	}
	return it->second.volume;
}

const prices_map& OrderBook::getBuyOrders() const {
	return buy_orders;
}
const prices_map& OrderBook::getSellOrders() const {
	return sell_orders;
}
