#pragma once

enum Side { BUY, SELL };

struct PriceLevel {
	double price;
	double volume;
public:
	PriceLevel(double price, double volume): price(price), volume(volume) {};
};
