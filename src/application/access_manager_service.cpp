#include "access_manager_service.hpp"
#include "access_manager.hpp"
#include "domain/access_manager_logic.hpp"
#include "domain/json.hpp"

AccessManagerService::AccessManagerService(ThreadSafePtr<PubNub> pubnub, Pubnub::String auth_key):
    pubnub(pubnub),
    auth_key(auth_key)
{}

bool AccessManagerService::can_i(Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const Pubnub::String& resource_name) const {
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

    return AccessManagerLogic::can_i(permission, resource_type, json_token, resource_name);
}

Pubnub::String AccessManagerService::parse_token(const Pubnub::String auth_key) const 
{
    auto pubnub_handle = this->pubnub->lock();
    return pubnub_handle->parse_token(auth_key);
}

void AccessManagerService::set_auth_token(const Pubnub::String token) const
{
    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->set_auth_token(token);
}

int AccessManagerService::set_pubnub_origin(const Pubnub::String origin) const
{
    auto pubnub_handle = this->pubnub->lock();
    return pubnub_handle->set_pubnub_origin(origin);
}
