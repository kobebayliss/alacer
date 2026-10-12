#include <atomic>
#include <optional>
#include <cstring>
#include <memory.h>
#include <iostream>
#include <fstream>
#include <string>
#include "ixwebsocket/IXWebSocketMessage.h"
#include <rapidjson/document.h>
#include <string_view>
#include "EventHandler.hpp"

static std::optional<OrderState> parseState(std::string_view s) {
	if (s == "NEW") return OrderState::NEW;
	if (s == "PARTIALLY_FILLED") return OrderState::PARTIALLY_FILLED;
	if (s == "FILLED") return OrderState::FILLED;
	if (s == "CANCELED") return OrderState::CANCELED;
	if (s == "REJECTED") return OrderState::REJECTED;
	if (s == "EXPIRED" || s == "EXPIRED_IN_MATCH") return OrderState::EXPIRED;
	return std::nullopt;
}

namespace EventHandler {
	void handleDepthUpdate(const ix::WebSocketMessagePtr& msg, SPSCQueue<RawMessage, CAPACITY>& eventQueue) {
		if (msg->type != ix::WebSocketMessageType::Message) {
			return;
		}
		RawMessage raw{};
		raw.length = std::min(msg->str.size(), sizeof(raw.data));
		memcpy(raw.data, msg->str.data(), raw.length);
		if (!eventQueue.try_push(raw)) {
			std::cout << "QUEUE FULL: DROPPING MESSAGE.\n";
		}
	}
	void handleUserDataUpdate(const ix::WebSocketMessagePtr& msg, SPSCQueue<OrderUpdate, CAPACITY>& updateQueue, std::ofstream& outputFile) { 
		std::cout << "MESSAGED RECIEVED\n";
		if (msg->type != ix::WebSocketMessageType::Message) return;
		rapidjson::Document doc;
		doc.Parse(msg->str.c_str());
		if (doc.HasParseError() || !doc.IsObject()) {
			std::cout << "PARSE ERROR WHEN RECIEVING UPDATE\n";
			return;
		}
		if (!doc.HasMember("event") || !doc["event"].IsObject()) return;  // filters subscribe ack etc.
		const rapidjson::Value& eventDetails = doc["event"];
		if (!eventDetails.HasMember("e") || std::string_view(eventDetails["e"].GetString()) != "executionReport") return;
		std::cout << "MESSAGED RECIEVED AFTER\n";

		OrderUpdate u{};
		const rapidjson::Value& clientOrderId = eventDetails["c"];
		uint32_t n;
		if (CLIENT_ORDER_ID_LENGTH - 1 <= clientOrderId.GetStringLength()) {
			n = CLIENT_ORDER_ID_LENGTH - 1;
		} else {
			n = clientOrderId.GetStringLength();
		}
		memcpy(u.clientOrderId, clientOrderId.GetString(), n);
		auto state = parseState(eventDetails["X"].GetString());
		if (!state) {
			std::cout << "UNKNOWN ORDER STATUS: " << eventDetails["X"].GetString() << '\n';
			return;
		}
		u.newState = *state;
		u.filledPrice = std::stod(eventDetails["L"].GetString());
		u.totalFilledVolume = std::stod(eventDetails["z"].GetString());
		u.timestamp = eventDetails["E"].GetUint64();
		// add to update stream queue (consumed by OMS thread)
		outputFile << u.clientOrderId << '\n' << eventDetails["X"].GetString() << '\n' << u.filledPrice << '\n' << u.totalFilledVolume << '\n' << u.timestamp << std::endl;
		if (!updateQueue.try_push(u)) {
			std::cout << "UPDATE QUEUE FULL: DROPPING UPDATE.\n";
		}
	}
}
