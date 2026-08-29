#pragma once
#include <atomic>
#include <cpr/ssl_options.h>
#include "../book/OrderBook.hpp"
#include "EventHandler.hpp"
#include <rapidjson/document.h>

uint64_t bookUpdater(SPSCQueue<RawMessage, CAPACITY>& queue, OrderBook& ob, std::atomic<bool>& running, uint64_t last_update_id, std::ofstream* outputfile = nullptr);
void add_to_orderbook(OrderBook& ob, const rapidjson::Value& orders, Side side);
