#include <stdexcept>
#include "chat.hpp"
#include "nlohmann/json.hpp"

extern "C" {
#include "core/pubnub_objects_api.h"
#include "core/pubnub_objects_api.h"
}

using namespace Pubnub;
using json = nlohmann::json;

Chat::Chat(const String publish_key, const String subscribe_key, const String user_id) : 
    pubnub(publish_key, subscribe_key, user_id) {}

void Chat::publish_message(String channel, String message)
{
    publish_message(channel, message);
}

Channel Chat::create_public_conversation(String channel_id, ChatChannelData channel_data)
{
    channel_data.type = "public";
    return create_channel(channel_id, channel_data);
}

Channel Chat::update_channel(String channel_id, ChatChannelData channel_data)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to update channel, channel_id is empty");
    }

    Channel channel_obj(*this, channel_id, channel_data);
    channel_obj.update(channel_data);

    return channel_obj;
}

Channel Chat::get_channel(String channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to get channel, channel_id is empty");
    }

    String channel_response = this->pubnub.get_channel_metadata(channel_id);

    json response_json = json::parse(channel_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channel, response is incorrect");
    }

    String channel_data_string = static_cast<Pubnub::String>(response_json["Data"]);

    return Channel(*this, channel_id, channel_data_string);
}

std::vector<Channel> Chat::get_channels(String include, int limit, String start, String end)
{
    String channels_response = this->pubnub.get_all_channels_metadata(include, limit, start, end);

    json response_json = json::parse(channels_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channels, response is incorrect");
    }

    json channel_data_array_json = response_json["Data"];
    std::vector<Channel> Channels;
   
   for (auto& element : channel_data_array_json)
   {
        Channel channel_obj(*this, String(element["id"]), String(element));
        Channels.push_back(channel_obj);
   }

    return Channels;
}

void Chat::delete_channel(String channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to delete channel, channel_id is empty");
    }

    this->pubnub.remove_channel_metadata(channel_id);
}

User Chat::create_user(String user_id, ChatUserData user_data)
{
    //Make sure user with this ID doesn't exist
    get_user(user_id);

    User created_user(*this, user_id, user_data);
    this->pubnub.set_user_metadata(user_id, created_user.user_data_to_json(user_id, user_data));

    return created_user;
}

User Chat::get_user(String user_id)
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to get user, user_id is empty");
    }

    auto channel_response = this->pubnub.get_user_metadata(user_id);

    json response_json = json::parse(channel_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("Failed to get user, response json can't be parsed");
    }

    String user_data_string = static_cast<Pubnub::String>(response_json["Data"]);
    User user_obj(*this, user_id, user_data_string);

    return user_obj;
}

std::vector<User> Chat::get_users(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end)
{
    String users_response = this->pubnub.get_all_user_metadata(include, limit, start, end);

    json response_json = json::parse(users_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channels, response is incorrect");
    }

    json user_data_array_json = response_json["Data"];
    std::vector<User> users;
   
   for (auto& element : user_data_array_json)
   {
        User user_obj(*this, String(element["id"]), String(element));
        users.push_back(user_obj);
   }

    return users;
}

User Chat::update_user(String user_id, ChatUserData user_data)
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to update user, user_id is empty");
    }

    User user_obj(*this, user_id, user_data);
    user_obj.update(user_data);

    return user_obj;
}

void Chat::delete_user(String user_id)
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to delete channel, channel_id is empty");
    }

    this->pubnub.remove_user_metadata(user_id);
}

void Chat::set_restrictions(String in_user_id, String in_channel_id, PubnubRestrictionsData restrictions)
{
    if(in_user_id.empty())
    {
        throw std::invalid_argument("Failed to set restrictions, user_id is empty");
    }
    if(in_channel_id.empty())
    {
        throw std::invalid_argument("Failed to set restrictions, channel_id is empty");
    }

	//Restrictions are held in new channel with ID: PUBNUB_INTERNAL_MODERATION_{ChannelName}
	String restrictions_channel = internal_moderation_prefix + in_channel_id;

	//Lift restrictions
	if(!restrictions.ban && !restrictions.mute)
	{
		String remove_member_string = String("[{\"uuid\": {\"id\": \"") + in_user_id + String("\"}}]");
        this->pubnub.remove_members(restrictions_channel, in_user_id);
		String event_payload_string = String("{\"channelId\": \"") + restrictions_channel + String("\", \"restriction\": \"lifted\", \"reason\": \"") + restrictions.reason + String("\"}");
        emit_chat_event(pubnub_chat_event_type::PCET_MODERATION, in_user_id, event_payload_string);
		return;
	}

	//Ban or mute the user
	String params_string = String("{\"ban\": ") + bool_to_string(restrictions.ban) + String(", \"mute\": ") + bool_to_string(restrictions.mute) + String(", \"reason\": \"") + restrictions.reason + String("\"}");
	String set_members_string = String("[{\"uuid\": {\"id\": \"") + this->pubnub.get_user_id() + String("\"}, \"custom\": ") + params_string + String("}]");
    this->pubnub.set_members(restrictions_channel, set_members_string);
    String restriction_text;
    restrictions.ban ? restriction_text = "banned" : "muted";
	String event_payload_string = String("{\"channelId\": \"") + restrictions_channel + String("\", \"restriction\": \"lifted") + restriction_text + String("\", \"reason\": \"") + restrictions.reason + String("\"}");
    emit_chat_event(pubnub_chat_event_type::PCET_MODERATION, in_user_id, event_payload_string);
}


PubnubRestrictionsData Chat::get_channel_restrictions(Pubnub::String in_user_id, Pubnub::String in_channel_id, int limit, String start, String end)
{
    String full_channel_id = internal_moderation_prefix + in_channel_id;

    String get_restrictions_response = this->pubnub.get_channel_members(full_channel_id, "totalCount,custom", limit, start, end);

    json response_json = json::parse(get_restrictions_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channel restrictions, response is incorrect");
    }

    json response_data_json = response_json["data"];
    PubnubRestrictionsData FinalRestrictionsData;

   for (auto& element : response_data_json)
   {
        //Find restrictions data for requested channel
        if(String(element["uuid"]["id"]) == in_user_id)
        {
            if(element["custom"]["ban"] == true)
            {
                FinalRestrictionsData.ban = true;
            }
            if(element["custom"]["mute"] == true)
            {
                FinalRestrictionsData.mute = true;
            }
            FinalRestrictionsData.reason = String(element["custom"]["reason"]);
            break;
        }
   }

   return FinalRestrictionsData;
}

std::vector<String> Chat::where_present(String user_id)
{
    String where_now_response = this->pubnub.where_now(user_id);

    json response_json = json::parse(where_now_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get where present, response is incorrect");
    }

    json response_payload_json = response_json["payload"];
    json channels_array_json = response_payload_json["channels"];

    std::vector<String> channel_ids;
   
    for (json::iterator it = channels_array_json.begin(); it != channels_array_json.end(); ++it) 
    {
        channel_ids.push_back(static_cast<String>(*it));
    }
    
    return channel_ids;
}

std::vector<String> Chat::who_is_present(String channel_id)
{
    String here_now_response = this->pubnub.here_now(channel_id);

    json response_json = json::parse(here_now_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get who is present, response is incorrect");
    }

    json uuids_array_json = response_json["uuids"];

    std::vector<String> user_ids;
   
    for (json::iterator it = uuids_array_json.begin(); it != uuids_array_json.end(); ++it) 
    {
        user_ids.push_back(static_cast<String>(*it));
    }
    
    return user_ids;
}

bool Chat::is_present(Pubnub::String user_id, Pubnub::String channel_id)
{
    std::vector<String> channels = where_present(user_id);
    int count = std::count(channels.begin(), channels.end(), channel_id);
    return count > 0;
}

void Chat::subscribe_to_channel(String channel_id)
{
    this->pubnub.subscribe_to_channel(channel_id);
}

void Chat::unsubscribe_from_channel(String channel_id)
{
    auto messages = this->pubnub.unsubscribe_from_channel_and_get_last_messages(channel_id);
    for(auto message : messages)
    {
        // TODO: callbacks
    }
}

Channel Chat::create_channel(String channel_id, ChatChannelData channel_data)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to create channel, channel_id is empty");
    }

    Channel channel_obj(*this, channel_id, channel_data);
    auto channel_data_json = channel_obj.channel_data_to_json(channel_id, channel_data);

    this->pubnub.set_channel_metadata(channel_id, channel_data_json);

    return channel_obj;
}

void Chat::emit_chat_event(pubnub_chat_event_type chat_event_type, String channel_id, String payload)
{
    //Payload is in form of Json: {"param1": "param1value", "param2": "param2value" ... }. So in order to get just parameters, we remove first and last curl bracket
	String payload_parameters = payload;
    payload_parameters.erase(0, 1);
	payload_parameters.erase(payload_parameters.length() - 1);
	String event_message = String("{") + payload_parameters + String(", \"type\": \"") + get_string_from_event_type(chat_event_type) = String("\"}");

    this->pubnub.publish(channel_id, event_message);
}

String Chat::get_string_from_event_type(pubnub_chat_event_type chat_event_type)
{
    switch(chat_event_type)
	{
	case pubnub_chat_event_type::PCET_TYPING:
		return "typing";
	case pubnub_chat_event_type::PCET_REPORT:
		return "report";
	case pubnub_chat_event_type::PCET_RECEPIT:
		return "receipt";
	case pubnub_chat_event_type::PCET_MENTION:
		return "mention";
	case pubnub_chat_event_type::PCET_INVITE:
		return "invite";
	case pubnub_chat_event_type::PCET_CUSTOM:
		return "custom";
	case pubnub_chat_event_type::PCET_MODERATION:
		return "moderation";
	}
	return "custom";
}
