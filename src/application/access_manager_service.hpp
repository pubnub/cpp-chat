#ifndef PN_CHAT_ACCESS_MANAGER_SERVICE_HPP
#define PN_CHAT_ACCESS_MANAGER_SERVICE_HPP

#include "application/chat_service.hpp"
#include "domain/access_manager_logic.hpp"
#include "access_manager.hpp"
#include "infra/pubnub.hpp"
#include "infra/sync.hpp"

class AccessManagerService {
    public:
        AccessManagerService(ThreadSafePtr<PubNub> pubnub);

        bool can_i(Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const Pubnub::String& resource_name) const; 

        Pubnub::String parse_token(const Pubnub::String auth_key) const;
        void set_auth_token(const Pubnub::String token) const;
        int set_pubnub_origin(const Pubnub::String origin) const;

    private:
        ThreadSafePtr<PubNub> pubnub;
};

#endif // PN_CHAT_ACCESS_MANAGER_SERVICE_HPP
