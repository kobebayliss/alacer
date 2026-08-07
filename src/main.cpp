#include <iomanip>
#include <iostream>
#include "OrderBook.hpp"
#include "OrderBookBuilder.hpp"

int main() {
	OrderBook ob = build_initial_orderbook();
	const size_t x = 16;
	std::vector<double> top_x = ob.get_top_k_prices(x, BUY);
	for (size_t i{0}; i < x; i++) {
		std::cout << std::fixed << std::setprecision(6) << "top x orders: " << top_x[i] << std::endl;
	}
	return 0;
}
