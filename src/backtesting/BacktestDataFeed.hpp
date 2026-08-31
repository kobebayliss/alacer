#pragma once
#include "../types/SPSCQueue.hpp"
#include "../types/RawMessage.hpp"

std::unique_ptr<std::string> getBookData(FILE* file);
size_t backtestJsonFile(FILE* file, SPSCQueue<RawMessage, CAPACITY>& eventQueue);
