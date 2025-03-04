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

        Pubnub::String parse_token(const Pubnub::String auth_key) const;
        void set_auth_token(const Pubnub::String token) const;
        int set_pubnub_origin(const Pubnub::String origin) const;
        Pubnub::String grant_token(const Pubnub::String permission_object) const;
        Pubnub::String grant_token_permission_struct_to_string(const Pubnub::GrantTokenPermissionObject permission_object_struct);

        Json add_channel_permissions_to_json(std::vector<Pubnub::String> channels, std::vector<Pubnub::ChannelPermissions> channel_permissions);
	    Json add_channel_group_permissions_to_json(std::vector<Pubnub::String> channel_groups, std::vector<Pubnub::ChannelGroupPermissions> channel_group_permissions);
	    Json add_user_permissions_to_json(std::vector<Pubnub::String>users, std::vector<Pubnub::UserPermissions> user_permissions);

    private:
        Pubnub::String auth_key;
        ThreadSafePtr<PubNub> pubnub;
};

#endif // PN_CHAT_ACCESS_MANAGER_SERVICE_HPP
