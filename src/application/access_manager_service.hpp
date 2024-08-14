#ifndef PN_CHAT_ACCESS_MANAGER_SERVICE_HPP
#define PN_CHAT_ACCESS_MANAGER_SERVICE_HPP

#include "application/chat_service.hpp"
#include "domain/access_manager_logic.hpp"
#include "access_manager.hpp"
#include "infra/pubnub.hpp"
#include "infra/sync.hpp"

class AccessManagerService {
    public:
        AccessManagerService(ThreadSafePtr<PubNub> pubnub, Pubnub::String auth_key);

        bool can_i(Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const Pubnub::String& resource_name) const; 

    private:
        Pubnub::String auth_key;
        ThreadSafePtr<PubNub> pubnub;
};

#endif // PN_CHAT_ACCESS_MANAGER_SERVICE_HPP
