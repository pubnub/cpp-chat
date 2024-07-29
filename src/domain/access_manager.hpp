#ifndef PN_CHAT_ACCESS_MANAGER_HPP
#define PN_CHAT_ACCESS_MANAGER_HPP

#include "domain/json.hpp"
#include "string.hpp"

namespace AccessManager {

    enum class ResourceType { UUIDS, CHANNELS };
    enum class Permission { READ, WRITE, MANAGE, DELETE, GET, JOIN, UPDATE };

    bool can_i(Permission permission, ResourceType resource_type, const Json& token, const Pubnub::String& resource_name);
};

#endif // PN_CHAT_ACCESS_MANAGER_HPP
