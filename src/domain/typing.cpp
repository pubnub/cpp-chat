#include "typing.hpp"
#include "domain/json.hpp"
#include <optional>

void Typing::start() {
    this->sent = true;
}

void Typing::stop() {
    this->sent = false;
}

bool Typing::is_sent() const {
    return this->sent;
}

void Typing::add_typing_indicator(const Pubnub::String& user_id) {
    this->typing_indicators.push_back(user_id);
}

void Typing::remove_typing_indicator(const Pubnub::String& user_id) {
    this->typing_indicators.erase(
            std::remove(this->typing_indicators.begin(), this->typing_indicators.end(), user_id),
            this->typing_indicators.end()
    );
}

const std::vector<Pubnub::String>& Typing::get_typing_indicators() const {
    return this->typing_indicators;
}

bool Typing::contains_typing_indicator(const Pubnub::String& user_id) const {
    return std::find(this->typing_indicators.begin(), this->typing_indicators.end(), user_id) != this->typing_indicators.end();
}

Pubnub::String Typing::payload(const Pubnub::String& user_id, bool is_typing) {
    Pubnub::String typing = is_typing ? "true" : "false";

    return "{\"value\": " + typing +", \"user_id\": \"" + user_id + "\"}";
}

std::optional<std::pair<Typing::UserId, bool>> Typing::typing_user_from_payload(const Json& payload) {
    auto json = Json::parse(payload);

    if(!json.contains("userId")) {
        return std::nullopt;
    }

    if(!json.contains("value")) {
        return std::nullopt;
    }

    return std::make_pair(json.get_string("userId").value(), json.get_bool("value").value());
}
