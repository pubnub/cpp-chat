#include "json.hpp"

Json::Json(Pubnub::String json):
    json(nlohmann::json::parse(json))
{}

Json::Json(nlohmann::json json):
    json(json)
{}

Json Json::parse(Pubnub::String json) {
    return Json(json);
}

Json Json::operator[](Pubnub::String key) const {
    return Json(json[key]);
}

Json Json::operator[](int index) const {
    return Json(json[index]);
}

// TODO: I'm not sure if it will work
Json Json::operator=(Pubnub::String json) {
    this->json = nlohmann::json::parse(json);
    return *this;
}

Json::operator Pubnub::String() const {
    return static_cast<std::string>(this->json);
}

bool Json::contains(Pubnub::String key) const {
    return this->json.contains(key);
}

bool Json::is_null() const {
    return this->json.is_null();
}

Pubnub::String Json::dump() const {
    return this->json.dump();
}

std::optional<Pubnub::String> Json::get_string(Pubnub::String key) const {
    if(this->json.contains(key) && this->json[key].is_string() && !this->json[key].is_null())
    {
        return std::optional(this->json[key].get<std::string>());
    }

    return std::nullopt;
}

void Json::insert_or_update(Pubnub::String key, Json value) {
    this->json[key] = value.json;
}

void Json::insert_or_update(Pubnub::String key, Pubnub::String value) {
    this->json[key] = value.c_str();
}

Json::Iterator Json::begin() {
    return Json::Iterator(this->json.begin());
}

Json::Iterator Json::end() {
    return Json::Iterator(this->json.end());
}

Json::Iterator::Iterator(nlohmann::json::iterator it):
    it(it)
{}

Json::Iterator Json::Iterator::operator++() {
    return Json::Iterator(++this->it);
}

bool Json::Iterator::operator!=(const Json::Iterator& other) const {
    return this->it != other.it;
}

Json Json::Iterator::operator*() const {
    return Json(*this->it);
}

Json Json::Iterator::value() const {
    return Json(this->it.value());
}

Pubnub::String Json::Iterator::key() const {
    return this->it.key();
}