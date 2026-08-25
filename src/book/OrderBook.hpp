#pragma once
#include "PriceLevel.hpp"
#include <condition_variable>
#include <vector>
#include <map>
#include <mutex>
#include <string>
#include <optional>

typedef std::map<double, PriceLevel> prices_map;
class OrderBook {
	mutable std::mutex mtx;
	prices_map buy_orders;
	prices_map sell_orders;

public:
	std::string instrument;
	std::atomic<bool> updated;

	explicit OrderBook(std::string instrument);
	// rule of 5 functions
	~OrderBook();
	OrderBook (const OrderBook& other);
	OrderBook& operator=(const OrderBook& other);
	OrderBook (OrderBook&& other) noexcept;
	OrderBook& operator=(OrderBook&& other) noexcept;

	// O(log n)
	void apply_delta(double price, double quantity, Side side);

	// O(1)
	std::pair<double, double> get_top_level(Side side) const;

	// O(k)
	std::vector<std::pair<double, double>> get_top_k_levels(size_t k, Side side) const;

	// O(log n) - can be made O(1)
	double get_volume_at_price(double price, Side side) const;

	// O(1)
	void notify_update();

	// O(1)
	void clear();
};
