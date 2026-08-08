#pragma once
#include <algorithm>
#include <utility>
#include <vector>
#include <format>
#include <stdexcept>
#include <map>
#include "Order.hpp"

typedef std::map<double, PriceLevel> prices_map;
class OrderBook {
	prices_map buy_orders;
	prices_map sell_orders;
	std::unordered_map<size_t, std::unique_ptr<Order>> orders;
	void remove_from_price_level(Order* order, PriceLevel& price_level, prices_map& map, prices_map::iterator level_it) const {
		if (order->prev) {
			order->prev->next = order->next;
		} else {
			price_level.head = order->next;
		}
		if (order->next) {
			order->next->prev = order->prev;
		} else {
			price_level.tail = order->prev;
		}
		price_level.volume -= order->quantity;
		order->next = nullptr;
		order->prev = nullptr;
		if (price_level.volume == 0) {
			map.erase(level_it);
		}
	}

public:
	// O(log n)
	void add_order(double price, double quantity, Side side) {
		std::unique_ptr<Order> order_ptr = std::make_unique<Order>(price, quantity, side, nullptr, nullptr);
		Order* order = order_ptr.get();
		orders[order->id] = std::move(order_ptr);
		prices_map& map = (side == BUY) ? buy_orders : sell_orders;
		auto it = map.find(price);
		if (it == map.end()) {
			// new price level
			map.try_emplace(price, price, quantity, order, order);
		} else {
			// existing price level
			PriceLevel& price_level = it->second;
			price_level.volume += quantity;
			Order* tail = price_level.tail;
			tail->next = order;
			order->prev = tail;
			price_level.tail = order;
		}
	}
	// O(1)
	std::pair<double, double> get_top_level(Side side) const {
		if (side == BUY) {
			auto it = buy_orders.rbegin();
			return {it->first, it->second.volume};
		}
		auto it = sell_orders.begin();
		return {it->first, it->second.volume};
	}
	// O(log n)
	void cancel_order(size_t id) {
		auto it = orders.find(id);
		if (it == orders.end()) return;
		Order* order = (it->second).get();
		prices_map& map = (order->side == BUY) ? buy_orders : sell_orders;
		auto level_it = map.find(order->price);
		PriceLevel& price_level = level_it->second;
		remove_from_price_level(order, price_level, map, level_it);
		orders.erase(it);
	}
	// O(log n)
	void update_order(size_t id, double new_price, double new_quantity) {
		auto order_it = orders.find(id);
		if (order_it == orders.end()) return;
		Order* order = order_it->second.get();
		prices_map& map = (order->side == BUY) ? buy_orders : sell_orders;
		if (order->price == new_price) {
			if (order->quantity == new_quantity) return;
			PriceLevel& price_level = map.find(order->price)->second;
			price_level.volume -= order->quantity;
			price_level.volume += new_quantity;
			order->quantity = new_quantity;
			return;
		}

		auto old_it = map.find(order->price);
		PriceLevel& old_price_level = old_it->second;
		remove_from_price_level(order, old_price_level, map, old_it);
		order->price = new_price;
		order->quantity = new_quantity;
		auto it = map.find(new_price);

		if (it == map.end()) {
			order->prev = nullptr;
			order->next = nullptr;
			map.try_emplace(new_price, new_price, new_quantity, order, order);
		} else {
			PriceLevel& new_price_level = it->second;
			new_price_level.volume += new_quantity;
			Order* tail = new_price_level.tail;
			tail->next = order;
			order->prev = tail;
			order->next = nullptr;
			new_price_level.tail = order;
		}
	}
	// O(log n) - can be made O(1)
	double get_quantity_at_price(double price, Side side) const {
		const prices_map& map = (side == BUY) ? buy_orders : sell_orders;
		auto it = map.find(price);
		if (it == map.end()) {
			throw std::invalid_argument(std::format("No orders at price {}", price));
		}
		return it->second.volume;
	}
	// O(k)
	std::vector<std::pair<double, double>> get_top_k_levels(size_t k, Side side) const { // returns pairs of price, quantity of top k levels
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
	// getters
	const prices_map& getBuyOrders() const {
		return buy_orders;
	}
	const prices_map& getSellOrders() const {
		return sell_orders;
	}
};

