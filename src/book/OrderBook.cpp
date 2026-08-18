#include <format>
#include <mutex>
#include <stdexcept>
#include "OrderBook.hpp"

OrderBook::OrderBook(std::string instrument) : instrument(std::move(instrument)) {};
OrderBook::~OrderBook() = default;

OrderBook::OrderBook(const OrderBook& other) {
	std::lock_guard lock(other.mtx);
	buy_orders = other.buy_orders;
	sell_orders = other.sell_orders;
	instrument = other.instrument;
}

OrderBook& OrderBook::operator=(const OrderBook& other) {
	if (this == &other) return *this;
	std::scoped_lock lock(mtx, other.mtx);
	buy_orders = other.buy_orders;
	sell_orders = other.sell_orders;
	instrument = other.instrument;
	return *this;
}

OrderBook::OrderBook(OrderBook&& other) noexcept {
	std::lock_guard lock(other.mtx);
	buy_orders = std::move(other.buy_orders);
	sell_orders = std::move(other.sell_orders);
	instrument = std::move(other.instrument);
}

OrderBook& OrderBook::operator=(OrderBook&& other) noexcept {
	if (this == &other) return *this;
	std::scoped_lock lock(this->mtx, other.mtx);
	buy_orders = std::move(other.buy_orders);
	sell_orders = std::move(other.sell_orders);
	instrument = std::move(other.instrument);
	return *this;
}

void OrderBook::apply_delta(double price, double volume, Side side) {
	prices_map& map = (side == BUY) ? buy_orders : sell_orders;
	std::lock_guard<std::mutex> lock(mtx);
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

std::optional<std::pair<double, double>> OrderBook::get_top_level(Side side) const {
	std::lock_guard lock(mtx);
	if (side == BUY) {
		if (buy_orders.empty()) return std::nullopt;
		auto it = buy_orders.rbegin();
		return std::make_pair(it->first, it->second.volume);
	} else {
		if (sell_orders.empty()) return std::nullopt;
		auto it = sell_orders.begin();
		return std::make_pair(it->first, it->second.volume);
	}
}

std::vector<std::pair<double, double>> OrderBook::get_top_k_levels(size_t k, Side side) const {
	const prices_map& map = (side == BUY) ? buy_orders : sell_orders;
	std::vector<std::pair<double, double>> result;
	std::lock_guard lock(mtx);
	k = std::min(k, map.size());
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
	std::lock_guard lock(mtx);
	auto it = map.find(price);
	if (it == map.end()) {
		throw std::invalid_argument(std::format("No orders at price {}", price));
	}
	return it->second.volume;
}

void OrderBook::clear() {
	std::lock_guard lock(mtx);
	buy_orders.clear();
	sell_orders.clear();
}
