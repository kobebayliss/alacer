#pragma once
#include "../feed/SPSCQueue.hpp"

std::unique_ptr<std::string> getBookData(FILE* file);
size_t backtestJsonFile(FILE* file, SPSCQueue<RawMessage, CAPACITY>& queue);
