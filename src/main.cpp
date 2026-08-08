#include "OrderBook.hpp"
#include "OrderBookBuilder.hpp"
#include "OrderBookDisplay.hpp"

int main() {
	OrderBook ob = build_initial_orderbook("BTCUSDT", 100);
	OrderBookDisplay::print(ob);
	OrderBookDisplay::printTopOfBook(ob);
	return 0;
}
