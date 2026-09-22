#pragma once
#include <string>

std::array<std::string, 2> getBinanceKeys();
std::string hmacSha256Hex(const std::string& key, const std::string& data);
int64_t getTimestampMillis();
std::string generateUserDataSignature(const std::string& queryString);
std::string generateRequestId();
std::string generateUserStreamRequest();
