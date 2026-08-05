#include <cpr/parameters.h>
#include <iostream>
#include <cpr/cpr.h>

int main() {
	for (int i = 0; i < 10; i++) {
		cpr::Response r = cpr::Get(
				cpr::Url{"https://testnet.binance.vision/api/v3/avgPrice"},
				cpr::Parameters{{"symbol", "BTCUSDT"}}
		);
		std::cout << "Body:" << std::endl << r.text;
		std::cin.get();
	}
	return 0;
}
