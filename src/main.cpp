#include <iostream>
#include <cpr/cpr.h>

int main() {
	cpr::Response r = cpr::Get(cpr::Url{"https://api.binance.com"});
	std::cout << "HELLO" << std::endl;
	return 0;
}
