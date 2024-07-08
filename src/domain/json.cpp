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

Json Json::operator[](Pubnub::String key) {
    return Json(json[key]);
}

Json Json::operator[](int index) {
    return Json(json[index]);
}

Json::operator Pubnub::String() const {
    return static_cast<std::string>(json);
}

bool Json::contains(Pubnub::String key) {
    return json.contains(key);
}
