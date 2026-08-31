#pragma once
#include <functional>

struct RawMessage {
	char data[131072];
	uint64_t length;
};

