#include "access_manager_service.hpp"
#include "access_manager.hpp"
#include "domain/access_manager_logic.hpp"
#include "domain/json.hpp"
extern "C" {
    #include <pubnub_grant_token_api.h>
}

AccessManagerService::AccessManagerService(ThreadSafePtr<PubNub> pubnub):
    pubnub(pubnub)
{}

bool AccessManagerService::can_i(Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const Pubnub::String& resource_name) const {
    
    Pubnub::String current_token = [this]{
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_current_auth_token();
    }();

    if (current_token.empty()) {
        return true;
    }

    auto parsed_token = [this, current_token]() {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->parse_token(current_token);
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

void AccessManagerService::set_secret_key(const Pubnub::String key) const
{
	auto pubnub_handle = this->pubnub->lock();
	pubnub_handle->set_secret_key(key);
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

Pubnub::String AccessManagerService::grant_token(Pubnub::String permission_object) const
{
    auto pubnub_handle = this->pubnub->lock();
    return pubnub_handle->grant_token(permission_object);
}
Pubnub::String AccessManagerService::grant_token_permission_struct_to_string(GrantTokenPermissionObjectInternal permission_object_struct) const
{
    //Make sure token data is provided in correct form. There must be the same amount of object and permissions or just one permission,
	//then one permission is used for every object
	
	if(permission_object_struct.channels.size() != permission_object_struct.channel_permissions.size() && permission_object_struct.channel_permissions.size() != 1)
	{
		throw std::runtime_error("Grant Token Structure To JsonString - Provide the same amount of channel_permissions and Channels (or only 1 ChannelPermission).");
		return "";
	}

	if(permission_object_struct.channel_groups.size() != permission_object_struct.channel_group_permissions.size() && permission_object_struct.channel_group_permissions.size() != 1)
	{
		throw std::runtime_error("Grant Token Structure To JsonString - Provide the same amount of channel_group_permissions and channel_groups (or only 1 channel_group_permissions).");
		return "";
	}

	if(permission_object_struct.users.size() != permission_object_struct.user_permissions.size() && permission_object_struct.user_permissions.size() != 1)
	{
		throw std::runtime_error("Grant Token Structure To JsonString - Provide the same amount of user_permissions and users (or only 1 user_permissions).");
		return "";
	}

	if(permission_object_struct.channel_patterns.size() != permission_object_struct.channel_pattern_permissions.size() && permission_object_struct.channel_pattern_permissions.size() != 1)
	{
		throw std::runtime_error("Grant Token Structure To JsonString - Provide the same amount of channel_pattern_permissions and ChannelsPatterns (or only 1 channel_pattern_permissions).");
		return "";
	}

	if(permission_object_struct.channel_group_patterns.size() != permission_object_struct.channel_group_pattern_permissions.size() && permission_object_struct.channel_group_pattern_permissions.size() != 1)
	{
		throw std::runtime_error("Grant Token Structure To JsonString - Provide the same amount of channel_group_pattern_permissions and channel_groupsPatterns (or only 1 channel_group_pattern_permissions).");
		return "";
	}

	if(permission_object_struct.user_patterns.size() != permission_object_struct.user_pattern_permissions.size() && permission_object_struct.user_pattern_permissions.size() != 1)
	{
		throw std::runtime_error("Grant Token Structure To JsonString - Provide the same amount of user_pattern_permissions and usersPatterns (or only 1 user_pattern_permissions).");
		return "";
	}
	
    Json permission_object_json;

	//Create Json objects with channels, groups, users permissions and their patterns
	Json channels_json_object = add_channel_permissions_to_json(permission_object_struct.channels, permission_object_struct.channel_permissions);
	Json channel_groups_json_object = add_channel_group_permissions_to_json(permission_object_struct.channel_groups, permission_object_struct.channel_group_permissions);
	Json users_json_object = add_user_permissions_to_json(permission_object_struct.users, permission_object_struct.user_permissions);
	Json channel_patterns_json_object = add_channel_permissions_to_json(permission_object_struct.channel_patterns, permission_object_struct.channel_pattern_permissions);
	Json channel_group_patterns_json_object = add_channel_group_permissions_to_json(permission_object_struct.channel_group_patterns, permission_object_struct.channel_group_pattern_permissions);
	Json user_patterns_json_object= add_user_permissions_to_json(permission_object_struct.user_patterns, permission_object_struct.user_pattern_permissions);

	//Add resources fields
	Json resources_json_object;
	if(permission_object_struct.channels.size() > 0)
	{
		resources_json_object.insert_or_update("channels", channels_json_object);
	}
	if(permission_object_struct.channel_groups.size() > 0)
	{
		resources_json_object.insert_or_update("groups", channel_groups_json_object);
	}
	if(permission_object_struct.users.size() > 0)
	{
		resources_json_object.insert_or_update("uuids", users_json_object);
	}

	//Add patterns fields
	Json patterns_json_object;
	if(permission_object_struct.channel_patterns.size() > 0)
	{
		patterns_json_object.insert_or_update("channels", channel_patterns_json_object);
	}
	if(permission_object_struct.channel_group_patterns.size() > 0)
	{
		patterns_json_object.insert_or_update("groups", channel_group_patterns_json_object);
	}
	if(permission_object_struct.user_patterns.size() > 0)
	{
		patterns_json_object.insert_or_update("uuids", user_patterns_json_object);
	}

	Json permission_object_struct_json_object;
	if(!resources_json_object.is_null())
	{
		permission_object_struct_json_object.insert_or_update("resources", resources_json_object);
	}
	if(!patterns_json_object.is_null())
	{
		permission_object_struct_json_object.insert_or_update("patterns", patterns_json_object);
	}

	Json permissions_json_object;
	permissions_json_object.insert_or_update("ttl", permission_object_struct.ttl_minutes);
	permissions_json_object.insert_or_update("authorized_uuid", permission_object_struct.authorized_user);
	permissions_json_object.insert_or_update("permissions", permission_object_struct_json_object);

	//Convert created Json object to string
	return permissions_json_object.dump();
}



Json AccessManagerService::add_channel_permissions_to_json(std::vector<Pubnub::String> channels, std::vector<Pubnub::ChannelPermissions> channel_permissions) const
{
	Json json_object;
	bool use_one_permission = channel_permissions.size() == 1;
	
	for(int i = 0; i < channels.size(); i++)
	{
		if(channels[i].empty())
		{
			continue;
		}
		
		//For permissions use the first index if this is the only valid index or corresponding channel index
		Pubnub::ChannelPermissions current_permissions;
		use_one_permission ? current_permissions = channel_permissions[0] : current_permissions = channel_permissions[i];

		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.read = current_permissions.read;
		ChPerm.write = current_permissions.write;
		ChPerm.del = current_permissions.del;
		ChPerm.get = current_permissions.get;
		ChPerm.update = current_permissions.update;
		ChPerm.manage = current_permissions.manage;
		ChPerm.join = current_permissions.join;
		ChPerm.create = false;
		int perm_bit_mask = pubnub_get_grant_bit_mask_value(ChPerm);

		json_object.insert_or_update(channels[i], perm_bit_mask);
	}
	
	return json_object;
}

Json AccessManagerService::add_channel_group_permissions_to_json(std::vector<Pubnub::String> channel_groups, std::vector<Pubnub::ChannelGroupPermissions> channel_group_permissions) const
{
	Json json_object;
	bool use_one_permission = channel_group_permissions.size() == 1;
	
	for(int i = 0; i < channel_groups.size(); i++)
	{
		if(channel_groups[i].empty())
		{
			continue;
		}
		
		//For permissions use the first index if this is the only valid index or corresponding channel index
		Pubnub::ChannelGroupPermissions current_permissions;
		use_one_permission ? current_permissions = channel_group_permissions[0] : current_permissions = channel_group_permissions[i];

		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.read = current_permissions.read;
		ChPerm.manage = current_permissions.manage;
		ChPerm.write = false;
		ChPerm.del = false;
		ChPerm.get = false;
		ChPerm.update = false;
		ChPerm.join = false;
		ChPerm.create = false;
		int perm_bit_mask = pubnub_get_grant_bit_mask_value(ChPerm);

		json_object.insert_or_update(channel_groups[i], perm_bit_mask);
	}
	
	return json_object;
}

Json AccessManagerService::add_user_permissions_to_json(std::vector<Pubnub::String>users, std::vector<Pubnub::UserPermissions> user_permissions) const
{
	Json json_object;
	bool use_one_permission = user_permissions.size() == 1;

	
	for(int i = 0; i < users.size(); i++)
	{
		if(users[i].empty())
		{
			continue;
		}
		
		//For permissions use the first index if this is the only valid index or corresponding channel index
		Pubnub::UserPermissions current_permissions;
		use_one_permission ? current_permissions = user_permissions[0] : current_permissions = user_permissions[i];

		//Create bit mask value from all permissions
		struct pam_permission ChPerm;
		ChPerm.del = current_permissions.del;
		ChPerm.get = current_permissions.get;
		ChPerm.update = current_permissions.update;
		ChPerm.read = false;
		ChPerm.write = false;
		ChPerm.manage = false;
		ChPerm.join = false;
		ChPerm.create = false;
		int perm_bit_mask = pubnub_get_grant_bit_mask_value(ChPerm);

		json_object.insert_or_update(users[i], perm_bit_mask);
	}

	return json_object;
}