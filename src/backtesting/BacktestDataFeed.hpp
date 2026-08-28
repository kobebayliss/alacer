#pragma once
#include "../feed/SPSCQueue.hpp"

std::unique_ptr<std::string> getBookData(FILE* file);
void BacktestJsonFile(FILE* file, SPSCQueue<RawMessage, CAPACITY>& queue);
