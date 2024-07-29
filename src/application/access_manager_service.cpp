#include "access_manager_service.hpp"
#include "domain/access_manager.hpp"
#include "domain/json.hpp"

AccessManagerService::AccessManagerService(ThreadSafePtr<PubNub> pubnub, Pubnub::String auth_key):
    pubnub(pubnub),
    auth_key(auth_key)
{}

bool AccessManagerService::can_i(AccessManager::Permission permission, AccessManager::ResourceType resource_type, const Pubnub::String& resource_name) const {
    if (this->auth_key.empty()) {
        return true;
    }

    auto parsed_token = [this]() {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->parse_token(this->auth_key);
    }();

    auto json_token = Json::parse(parsed_token);

    if (json_token.is_null()) {
        throw std::runtime_error("Invalid access token from auth");
    }

    if (!json_token.contains("res") && !json_token.contains("pat")) {
        // TODO: I'm not sure about that assumption
        return true;
    }

    return AccessManager::can_i(permission, resource_type, json_token, resource_name);
}
