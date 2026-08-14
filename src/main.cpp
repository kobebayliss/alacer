#include "book/OrderBook.hpp"
#include "book/OrderBookBuilder.hpp"
#include "book/OrderBookDisplay.hpp"

int main() {
	OrderBook ob = build_initial_orderbook("BTCUSDT", 100);
	OrderBookDisplay::print(ob);
	OrderBookDisplay::printTopOfBook(ob);
	return 0;
}
