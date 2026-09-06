#pragma once
#include "../types/OrderIntent.hpp"
#include "../types/SPSCQueue.hpp"

std::array<std::string, 2> getBinanceKeys();
std::string hmacSha256Hex(const std::string& key, const std::string& data);
int64_t getTimestampMillis();
void executionLoop(SPSCQueue<OrderIntent, CAPACITY>& orderQueue, std::atomic<bool>& running);
