#include "OrderBookDisplay.hpp"
#include <iostream>
#include <format>
#include <chrono>
#include <thread>

namespace {
	constexpr int PRICE_WIDTH = 12;
	constexpr int QUANTITY_WIDTH = 16;
	std::string formatLine(double bidPrice, double bidQuantity, double askPrice, double askQuantity) {
		return std::format(
			"{:>{}.2f}    {:>{}.8f}    |    {:>{}.2f}    {:>{}.8f}\n",
			bidPrice, PRICE_WIDTH, bidQuantity, QUANTITY_WIDTH,
			askPrice, PRICE_WIDTH, askQuantity, QUANTITY_WIDTH
		);
	}
	std::string formatHeader() {
		return std::format(
			"{:>{}}    {:>{}}    |    {:>{}}    {:>{}}\n",
			"BID PRICE", PRICE_WIDTH, "BID QTY", QUANTITY_WIDTH,
			"ASK PRICE", PRICE_WIDTH, "ASK QTY", QUANTITY_WIDTH
		);
	}
}

namespace OrderBookDisplay {
	const uint64_t DEPTH = 20;
	const uint64_t UPDATE_INTERVAL_MS = 500;
	void printLoop(const OrderBook& ob, std::atomic<bool>& running) {
		while (running) {
			std::cout << "\033[H\033[J";
			print(ob);
			std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_INTERVAL_MS));
		}
	}
	void print(const OrderBook& ob) {
		std::vector<std::pair<double, double>> bids = ob.get_top_k_levels(DEPTH, BUY);
		std::vector<std::pair<double, double>> asks = ob.get_top_k_levels(DEPTH, SELL);
		std::string ob_display;
		for (size_t i = 0; i < DEPTH; i++) {
			ob_display = ob_display + formatLine(bids[i].first, bids[i].second, asks[i].first, asks[i].second);
		}

		std::cout << formatHeader();
		std::cout << ob_display;
	}
	void printTopOfBook(const OrderBook& ob) {
		auto bid = ob.get_top_level(BUY);
		auto ask = ob.get_top_level(SELL);
		std::string top_display = formatLine(bid.first, bid.second, ask.first, ask.second);

		std::cout << formatHeader();
		std::cout << top_display;
	}
}
