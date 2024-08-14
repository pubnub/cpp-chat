#ifndef PN_CHAT_ACCESS_MANAGER_LOGIC_HPP
#define PN_CHAT_ACCESS_MANAGER_LOGIC_HPP

#include "domain/json.hpp"
#include "string.hpp"
#include "access_manager.hpp"

namespace AccessManagerLogic {
    bool can_i(Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const Json& token, const Pubnub::String& resource_name);
};

#endif // PN_CHAT_ACCESS_MANAGER_LOGIC_HPP
