#ifndef PN_CHAT_ACCESS_MANAGER_HPP
#define PN_CHAT_ACCESS_MANAGER_HPP

#include "string.hpp"
#include <memory>

class AccessManagerService;

namespace Pubnub {
    class Chat;

    class AccessManager {
        public:
        enum class ResourceType { UUIDS, CHANNELS };
        enum class Permission { READ, WRITE, MANAGE, DELETE, GET, JOIN, UPDATE };

        AccessManager() = default;
        ~AccessManager() = default;

        bool can_i(AccessManager::Permission permission, AccessManager::ResourceType resource_type, const Pubnub::String& resource_name) const; 

        private:
        AccessManager(std::shared_ptr<const AccessManagerService> access_manager_service);

        std::shared_ptr<const AccessManagerService> access_manager_service;

        friend class Chat;
    };
}

#endif // PN_CHAT_ACCESS_MANAGER_HPP
