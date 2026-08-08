#pragma once
#include <atomic>

enum Side { BUY, SELL };

struct Order {
	inline static std::atomic<size_t> next_id = 1;
	size_t id;
	double price;
	double quantity;
	Side side;
	Order* next;
	Order* prev;

public:
	Order(double price, double quantity, Side side, Order* next, Order* prev) {
		this->id = next_id++;
		this->price = price;
		this->quantity = quantity;
		this->side = side;
		this->next = next;
		this->prev = prev;
	}
};

struct PriceLevel {
	double price;
	size_t volume;
	Order* head;
	Order* tail;

public:
	PriceLevel(double price, size_t volume, Order* head, Order* tail) {
		this->price = price;
		this->volume = volume;
		this->head = head;
		this->tail = tail;
	}
};

