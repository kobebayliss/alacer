#pragma once
#include <string>

std::array<std::string, 2> getBinanceKeys();
std::string hmacSha256Hex(const std::string& key, const std::string& data);
int64_t getTimestampMillis();
std::string generateUserDataSignature(const std::string& queryString);
std::string generateRequestId();
std::array<std::string, 2> getBinanceEd25519Keys();
std::string signEd25519Base64(const std::string& privateKeyPath, const std::string& data);
std::string generateSessionLogonRequest();
std::string generateUserStreamRequest();
