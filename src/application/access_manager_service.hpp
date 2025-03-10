#ifndef PN_CHAT_ACCESS_MANAGER_SERVICE_HPP
#define PN_CHAT_ACCESS_MANAGER_SERVICE_HPP

#include "application/chat_service.hpp"
#include "domain/access_manager_logic.hpp"
#include "access_manager.hpp"
#include "infra/pubnub.hpp"
#include "infra/sync.hpp"
#include <vector>

struct GrantTokenPermissionObjectInternal
{
    int ttl_minutes = 0;
    Pubnub::String authorized_user = "";
    std::vector<Pubnub::String> channels;
    std::vector<Pubnub::ChannelPermissions> channel_permissions;
    std::vector<Pubnub::String> channel_groups;
    std::vector<Pubnub::ChannelGroupPermissions> channel_group_permissions;
    std::vector<Pubnub::String> users;
    std::vector<Pubnub::UserPermissions> user_permissions;
    std::vector<Pubnub::String> channel_patterns;
    std::vector<Pubnub::ChannelPermissions> channel_pattern_permissions;
    std::vector<Pubnub::String> channel_group_patterns;
    std::vector<Pubnub::ChannelGroupPermissions> channel_group_pattern_permissions;
    std::vector<Pubnub::String> user_patterns;
    std::vector<Pubnub::UserPermissions> user_pattern_permissions;

    GrantTokenPermissionObjectInternal() = default;

    GrantTokenPermissionObjectInternal(Pubnub::GrantTokenPermissionObject permission_object):
    ttl_minutes(permission_object.ttl_minutes),
    authorized_user(permission_object.authorized_user),
    channels(permission_object.channels.into_std_vector()),
    channel_permissions(permission_object.channel_permissions.into_std_vector()),
    channel_groups(permission_object.channel_groups.into_std_vector()),
    channel_group_permissions(permission_object.channel_group_permissions.into_std_vector()),
    users(permission_object.users.into_std_vector()),
    user_permissions(permission_object.user_permissions.into_std_vector()),
    channel_patterns(permission_object.channel_patterns.into_std_vector()),
    channel_pattern_permissions(permission_object.channel_pattern_permissions.into_std_vector()),
    channel_group_patterns(permission_object.channel_group_patterns.into_std_vector()),
    channel_group_pattern_permissions(permission_object.channel_group_pattern_permissions.into_std_vector()),
    user_patterns(permission_object.user_patterns.into_std_vector()),
    user_pattern_permissions(permission_object.user_pattern_permissions.into_std_vector()){};
};

class AccessManagerService {
    public:
        AccessManagerService(ThreadSafePtr<PubNub> pubnub, Pubnub::String auth_key);

        bool can_i(Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const Pubnub::String& resource_name) const; 

        void set_secret_key(const Pubnub::String secret_key) const;
        Pubnub::String parse_token(const Pubnub::String auth_key) const;
        void set_auth_token(const Pubnub::String token) const;
        int set_pubnub_origin(const Pubnub::String origin) const;
        Pubnub::String grant_token(Pubnub::String permission_object) const;
        Pubnub::String grant_token_permission_struct_to_string(GrantTokenPermissionObjectInternal permission_object_struct) const;

        Json add_channel_permissions_to_json(std::vector<Pubnub::String> channels, std::vector<Pubnub::ChannelPermissions> channel_permissions) const;
	    Json add_channel_group_permissions_to_json(std::vector<Pubnub::String> channel_groups, std::vector<Pubnub::ChannelGroupPermissions> channel_group_permissions) const;
	    Json add_user_permissions_to_json(std::vector<Pubnub::String>users, std::vector<Pubnub::UserPermissions> user_permissions) const;

    private:
        Pubnub::String auth_key;
        ThreadSafePtr<PubNub> pubnub;
};

#endif // PN_CHAT_ACCESS_MANAGER_SERVICE_HPP
