#include "access_manager_logic.hpp"
#include "access_manager.hpp"
#include "domain/json.hpp"
#include <regex>

extern "C" {
#include <pubnub_grant_token_api.h>
}

static int calculate_permission(Pubnub::AccessManager::Permission permission, int permission_value) {
    int result_mask = -1;

    // acording to the pubnub_grant_token_api.h file
    switch (permission) {
        case Pubnub::AccessManager::Permission::READ:
            result_mask = permission_value & 1;
            break;
        case Pubnub::AccessManager::Permission::WRITE:
            result_mask = permission_value & 2;
            break;
        case Pubnub::AccessManager::Permission::MANAGE:
            result_mask = permission_value & 4;
            break;
        case Pubnub::AccessManager::Permission::DELETE:
            result_mask = permission_value & 8;
            break;
        case Pubnub::AccessManager::Permission::GET:
            result_mask = permission_value & 32;
            break;
        case Pubnub::AccessManager::Permission::JOIN:
            result_mask = permission_value & 128;
            break;
        case Pubnub::AccessManager::Permission::UPDATE:
            result_mask = permission_value & 64;
            break;
    };

    return result_mask;
}

bool AccessManagerLogic::can_i(Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const Json& token, const Pubnub::String& resource_name) {
    auto test = token.dump();
    auto resources = token["res"];
    auto resource = resource_type == Pubnub::AccessManager::ResourceType::UUIDS ? resources["uuid"] : resources["chan"];

    // TODO: refactor a little bit to make it more readable
    if (!resource.is_null() && resource.contains(resource_name)) {
        int permission_value = resource.get_int(resource_name).value();
        
        int result_mask = calculate_permission(permission, permission_value);

        if (result_mask != -1) {
            return result_mask > 0;
        }
    }

    auto patterns = token["pat"];
    auto pattern = resource_type == Pubnub::AccessManager::ResourceType::UUIDS ? patterns["uuid"] : patterns["chan"];

    if (pattern.is_null()) {
        return false;
    }

    for (auto it = pattern.begin(); it != pattern.end(); ++it) {
        std::regex regex(it.key());

        if (std::regex_match(resource_name.c_str(), regex)) {
            int permission_value = it.value().as_int();

            int result_mask = calculate_permission(permission, permission_value);

            if (result_mask != -1) {
                return result_mask > 0;
            }
        }
    }

    return false;
}
