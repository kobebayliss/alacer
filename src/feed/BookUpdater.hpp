#pragma once
#include <atomic>
#include <cpr/ssl_options.h>
#include "EventHandler.hpp"
#include "../book/OrderBook.hpp"
#include <rapidjson/document.h>

void bookUpdater(SPSCQueue<RawMessage, CAPACITY>& queue, OrderBook& ob, std::atomic<bool>& running);
void add_to_orderbook(OrderBook& ob, const rapidjson::Value& orders, Side side);
