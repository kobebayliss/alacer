#include "OrderBookDisplay.hpp"
#include <iostream>

namespace {
	std::string formatLine(double price, double quantity) {
		return std::format("    {:>12.2f}    {:>16.8f}\n", price, quantity);
	}
}

namespace OrderBookDisplay {
	void print(const OrderBook& ob, size_t depth) {
		prices_map buy_orders = ob.getBuyOrders();
		prices_map sell_orders = ob.getSellOrders();
		size_t i = 0;
		std::string ob_display;
		for (auto it = buy_orders.rbegin(); i < depth && it != buy_orders.rend(); it++) {
			ob_display = ob_display + formatLine(it->first, it->second.head->quantity);
			i++;
		}
		std::cout << std::format("    {:>12}    {:>16}\n", "PRICE", "QUANTITY") << ob_display;
	}
	void printTopOfBook(const OrderBook& ob) {
		// print logic
	}
}
