#include "OrderBook.hpp"
#include "OrderBookBuilder.hpp"
#include "OrderBookDisplay.hpp"

int main() {
	OrderBook ob = build_initial_orderbook("BTCUSDT", 100);
	const size_t x = 16;
	std::vector<double> top_x = ob.get_top_k_prices(x, BUY);
	OrderBookDisplay::print(ob);
	return 0;
}
