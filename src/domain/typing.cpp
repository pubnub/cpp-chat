#include "typing.hpp"
#include "domain/json.hpp"
#include "string.hpp"
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
            std::remove_if(
                this->typing_indicators.begin(),
                this->typing_indicators.end(),
                [&user_id](const auto other) {return user_id == other;}),
            this->typing_indicators.end()
    );
}

const std::vector<Pubnub::String>& Typing::get_typing_indicators() const {
    return this->typing_indicators;
}

bool Typing::contains_typing_indicator(const Pubnub::String& user_id) const {
    return std::find_if(
            this->typing_indicators.begin(),
            this->typing_indicators.end(),
            [&user_id](const auto other) {return user_id == other;}
        ) != this->typing_indicators.end();
}

Pubnub::String Typing::payload(bool is_typing) {
    Pubnub::String typing = is_typing ? "true" : "false";

    return "{\"value\": " + typing + "}";
}

std::optional<bool> Typing::typing_value_from_event(const Pubnub::Event& event)
{
    if(event.payload.empty())
    {
        return std::nullopt;
    }
    Json payload = Json::parse(event.payload);

    if(!payload.contains("value")) {
        return std::nullopt;
    }

    return payload.get_bool("value").value();
}
