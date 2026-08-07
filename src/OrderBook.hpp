#include <iostream>
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
	void remove_from_price_level(Order* order, PriceLevel& price_level, prices_map& map) {
		if (order->next != nullptr) {
			Order* next_order = order->next;
			if (order->prev != nullptr) {
				Order* prev_order = order->prev;
				prev_order->next = next_order;
				next_order->prev = prev_order;
			} else {
				next_order->prev = nullptr;
				price_level.head = next_order;
			}
		} else if (order->prev != nullptr) {
			Order* prev_order = order->prev;
			prev_order->next = nullptr;
			price_level.tail = prev_order;
		}
		price_level.volume -= order->quantity;
		if (price_level.volume == 0) {
			map.erase(order->price);
		}
	}

public:
	void place_order(double price, size_t quantity, Side side) {
		std::unique_ptr<Order> order_ptr = std::make_unique<Order>(price, quantity, side, nullptr, nullptr);
		Order* order = order_ptr.get();
		orders[order->id] = std::move(order_ptr);
		prices_map& map = (side == BUY) ? buy_orders : sell_orders;
		auto it = map.find(price);
		if (it == map.end()) {
			// new price level
			map.emplace(price, PriceLevel(price, quantity, order, order));
		} else {
			// existing price level
			PriceLevel& price_level = it->second;
			price_level.volume += quantity;
			Order* tail = price_level.tail;
			tail->next = order;
			order->prev = tail;
			price_level.tail = order;
		}
		std::cout << "Your order is Order # : " << order->id << std::endl;
	}
	double get_top(Side side) {
		if (side == BUY) {
			return buy_orders.rbegin()->first;
		} else {
			return sell_orders.begin()->first;
		}
	}
	void cancel_order(size_t id) {
		auto it = orders.find(id);
		Order* order = (it->second).get();
		prices_map& map = (order->side == BUY) ? buy_orders : sell_orders;
		PriceLevel& price_level = map.find(order->price)->second;
		remove_from_price_level(order, price_level, map);
		order->next = nullptr;
		order->prev = nullptr;
		orders.erase(it);
	}
	void update_order(size_t id, double new_price, size_t new_quantity) {
		Order* order = (orders.find(id)->second).get();
		if (order->price == new_price && order->quantity == new_quantity)
			return;
		prices_map& map = (order->side == BUY) ? buy_orders : sell_orders;
		PriceLevel& old_price_level = map.find(order->price)->second;
		remove_from_price_level(order, old_price_level, map);
		order->price = new_price;
		order->quantity = new_quantity;
		auto it = map.find(new_price);
		if (it == map.end()) {
			order->prev = nullptr;
			order->next = nullptr;
			map.emplace(new_price, PriceLevel(new_price, new_quantity, order, order));
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
	size_t get_quantity_at_price(double price, Side side) {
		prices_map& map = (side == BUY) ? buy_orders : sell_orders;
		auto it = map.find(price);
		if (it == map.end()) {
			throw std::invalid_argument(std::format("No orders at price {}", price));
		}
		return it->second.volume;
	}
	std::vector<double> get_top_k_prices(int k, Side side) {
		prices_map& map = (side == BUY) ? buy_orders : sell_orders;
		std::vector<double> top_k_prices;
		if (side == BUY) {
			auto end = std::next(map.rbegin(), k);
			for (auto it = map.rbegin(); it != end; ++it) {
				top_k_prices.push_back(it->first);
			}
		} else if (side == SELL) {
			auto end = std::next(map.begin(), k);
			for (auto it = map.begin(); it != end; ++it) {
				top_k_prices.push_back(it->first);
			}
		}
		return top_k_prices;
	}
};

