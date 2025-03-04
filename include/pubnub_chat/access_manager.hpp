#ifndef PN_CHAT_ACCESS_MANAGER_HPP
#define PN_CHAT_ACCESS_MANAGER_HPP

#include "string.hpp"
#include <memory>
#include "vector.hpp"

class AccessManagerService;

namespace Pubnub {
    class Chat;


struct ChannelPermissions
{
	//Read permission. Applies to Subscribe, History, and Presence.
	bool read = false;
	//Write permission. Applies to Publish.
	bool write = false;
	//Delete permission. Applies to History and App Context. (delete is keyword in C++ so we use del)
	bool del = false;
	//Get permission. Applies to App Context.
	bool get = false;
	//Update permission. Applies to App Context.
	bool update = false;
	//Manage permission. Applies to Channel Groups and App Context.
	bool manage = false;
	//Join permission. Applies to App Context.
	bool join = false;
};


struct ChannelGroupPermissions
{
	//Read permission. Applies to presence and history access for the group.
	bool read = false;
	//Manage permission. Applies to modifying members of the group.
	bool manage = false;
};


struct UserPermissions
{
	//Delete permission. Allows deletion of user metadata.
	bool del = false;
	//Get permission. Allows retrieval of user metadata.
	bool get = false;
	//Update permission. Allows updating of user metadata.
	bool update = false;
};

    struct GrantTokenPermissionObject
    {
        //Time-To-Live (TTL) in minutes for the granted token.
        int ttl_minutes = 0;
        //The User that is authorized by this grant.
        Pubnub::String authorized_user = "";
        //List of channel names included in this grant.
        Pubnub::Vector<Pubnub::String> channels;
        //Permissions applied to the listed channels. Has to be either 1 or the same amount as Channels.
        Pubnub::Vector<ChannelPermissions> channel_permissions;
        //List of channel group names included in this grant.
        Pubnub::Vector<Pubnub::String> channel_groups;
        //Permissions applied to the listed channel groups. Has to be either 1 or the same amount as ChannelGroups.
        Pubnub::Vector<ChannelGroupPermissions> channel_group_permissions;
        //List of Users included in this grant.
        Pubnub::Vector<Pubnub::String> users;
        //Permissions applied to the listed Users. Has to be either 1 or the same amount as Users.
        Pubnub::Vector<UserPermissions> user_permissions;
        //List of channel name patterns included in this grant.
        Pubnub::Vector<Pubnub::String> channel_patterns;
        //Permissions applied to the listed channel name patterns. Has to be either 1 or the same amount as ChannelPatterns.
        Pubnub::Vector<ChannelPermissions> channel_pattern_permissions;
        //List of channel group name patterns included in this grant.
        Pubnub::Vector<Pubnub::String> channel_group_patterns;
        //Permissions applied to the listed channel group name patterns. Has to be either 1 or the same amount as ChannelGroupPatterns.
        Pubnub::Vector<ChannelGroupPermissions> channel_group_pattern_permissions;
        //List of User name patterns included in this grant.
        Pubnub::Vector<Pubnub::String> user_patterns;
        //Permissions applied to the listed User name patterns. Has to be either 1 or the same amount as UserPatterns.
        Pubnub::Vector<UserPermissions> user_pattern_permissions;
    };


    class AccessManager {
        public:
        enum class ResourceType { UUIDS, CHANNELS };
        enum class Permission { READ, WRITE, MANAGE, DELETE, GET, JOIN, UPDATE };

        PN_CHAT_EXPORT AccessManager() = default;
        PN_CHAT_EXPORT ~AccessManager() = default;

        PN_CHAT_EXPORT AccessManager(const AccessManager& other);
        PN_CHAT_EXPORT AccessManager& operator=(const AccessManager& other);


        PN_CHAT_EXPORT bool can_i(AccessManager::Permission permission, AccessManager::ResourceType resource_type, const Pubnub::String& resource_name) const; 
        
        PN_CHAT_EXPORT Pubnub::String parse_token(const Pubnub::String token) const;
        PN_CHAT_EXPORT void set_auth_token(const Pubnub::String token) const;
        PN_CHAT_EXPORT int set_pubnub_origin(const Pubnub::String origin) const;
        PN_CHAT_EXPORT Pubnub::String grant_token(Pubnub::String permission_object) const;

        private:
        AccessManager(std::shared_ptr<const AccessManagerService> access_manager_service);

        std::shared_ptr<const AccessManagerService> access_manager_service;

        friend class Chat;
    };
}

#endif // PN_CHAT_ACCESS_MANAGER_HPP
