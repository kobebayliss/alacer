#include <cpr/cpr.h>
#include "AccountData.hpp"
#include "TradeExecution.hpp"

std::string getAccountData() {
	auto keys = getBinanceKeys();
	const std::string apiKey = keys[0];
	const std::string secretKey = keys[1];
	std::string timestamp = std::to_string(getTimestampMillis());
	std::string queryString =
		"timestamp=" + timestamp +
		"&omitZeroBalances=true";

	std::string signature = hmacSha256Hex(secretKey, queryString);
	std::string fullUrl = "https://testnet.binance.vision/api/v3/account?" + queryString + "&signature=" + signature;

	cpr::Response r = cpr::Get(
	    cpr::Url{fullUrl},
	    cpr::Header{{"X-MBX-APIKEY", apiKey}}
	);
	return r.text;
}
