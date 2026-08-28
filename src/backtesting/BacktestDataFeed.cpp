#include "BacktestDataFeed.hpp"
#include "BacktestEventHandler.hpp"
#include "rapidjson/document.h"
#include <iostream>
#include <memory>

std::unique_ptr<std::string> getBookData(FILE* file) {
	std::string bookData;
	if (!file) {
		std::cerr << "Error: Could not open file.";
		return std::make_unique<std::string>(bookData);
	}
	// parse book data and build initial state
	char lineBuffer[65536];
	while (fgets(lineBuffer, sizeof(lineBuffer), file)) {
		bookData += lineBuffer;
		size_t len = bookData.size();
		if (len > 0 && bookData[len - 1] == '\n') {
			bookData.pop_back();  // strip trailing newline
			break;
		}
	}
	return std::make_unique<std::string>(bookData);
}


void backtestJsonFile(FILE* file, SPSCQueue<RawMessage, CAPACITY>& queue) {
	if (!file) {
		std::cerr << "Error: Could not open file.";
		return;
	}
	char lineBuffer[65536];
	rapidjson::Document document;
	while (fgets(lineBuffer, sizeof(lineBuffer), file)) {
		document.Parse(lineBuffer);
		if (document.HasParseError()) {
			std::cerr << "Skipping malformed line" << std::endl;
			continue;
		}
		BacktestEventHandler::handleMessage(document, queue);
	}
	fclose(file);
}
