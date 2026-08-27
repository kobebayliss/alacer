#include "Strategy.hpp"
#include "OrderIntent.hpp"
#include <atomic>
#include <iostream>
#include <fstream>

void strategyLoop(OrderBook &ob, std::atomic<bool> &running) {
	std::ofstream outputFile("data/imbalance7.txt");
	if (!outputFile.is_open()) {
		std::cerr << "Error: Could not open the file!" << std::endl;
		return;
	}
	while (running) {
		ob.updated.wait(false, std::memory_order_acquire);
		ob.updated.store(false, std::memory_order_relaxed);
		if (!running) [[unlikely]] break;

		const size_t k = 5;
		auto top_k_bids = ob.get_top_k_levels(k, BUY);
		auto top_k_asks = ob.get_top_k_levels(k, SELL);
		double bid_volume = top_k_bids.second;
		double ask_volume = top_k_asks.second;
		double imbalance = (bid_volume - ask_volume) / (bid_volume + ask_volume);
		outputFile << imbalance << '\n';
		outputFile.flush();
		std::cout << "IMBALANCE IS: " << imbalance << '\n';
	}
	outputFile.close();
}
