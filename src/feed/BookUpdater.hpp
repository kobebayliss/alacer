#pragma once
#include <atomic>
#include <cpr/ssl_options.h>
#include <rapidjson/document.h>
#include "../book/OrderBook.hpp"
#include "../types/RawMessage.hpp"
#include "../types/SPSCQueue.hpp"
#include "../types/Side.hpp"

uint64_t bookUpdater(SPSCQueue<RawMessage, CAPACITY>& eventQueue, OrderBook& ob, std::atomic<bool>& running, uint64_t last_update_id, std::ofstream* outputfile = nullptr);
void add_to_orderbook(OrderBook& ob, const rapidjson::Value& orders, Side side);
