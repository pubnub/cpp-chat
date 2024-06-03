#include <iostream>
#include <stdexcept>
#include "chat.hpp"
#include "nlohmann/json.hpp"

extern "C" {
#include "core/pubnub_alloc.h"
#include "core/pubnub_pubsubapi.h"
#include "core/pubnub_ntf_sync.h"
#include "core/pubnub_objects_api.h"
#include "core/pubnub_coreapi.h"
#include "core/pubnub_objects_api.h"
}

using namespace Pubnub;
using json = nlohmann::json;

void Chat::init(String in_publish_key, String in_subscribe_key, String in_user_id)
{
    publish_key = in_publish_key;
    subscribe_key = in_subscribe_key;
    user_id = in_user_id;
    ctx_pub = pubnub_alloc();
    ctx_sub = pubnub_alloc();

    if (NULL == ctx_pub) 
    {
        std::cout << "Failed to allocate Pubnub context\n" << std::endl;
        return;
    }

    pubnub_init(ctx_pub, publish_key, subscribe_key);
    pubnub_init(ctx_sub, publish_key, subscribe_key);
    pubnub_set_user_id(ctx_pub, user_id);
    pubnub_set_user_id(ctx_sub, user_id);
}

void Chat::deinit()
{
    if(ctx_pub)
    {
        pubnub_free(ctx_pub);
        ctx_pub = nullptr;
    }

    if(ctx_sub)
    {
        pubnub_free(ctx_sub);
        ctx_sub = nullptr;
    }
}

void Chat::publish_message(String channel, String message)
{
    publish_message(channel, message);
}

Channel Chat::create_public_conversation(String channel_id, ChatChannelData channel_data)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to create public conversation, channel_id is empty");
    }

    Channel channel_obj;
    channel_obj.init(this, channel_id, channel_data);
    pubnub_set_channelmetadata(ctx_pub, channel_id, NULL, channel_obj.channel_data_to_json(channel_id, channel_data));

    return channel_obj;
}

Channel Chat::update_channel(String channel_id, ChatChannelData channel_data)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to update channel, channel_id is empty");
    }

    Channel channel_obj;
    channel_obj.init(this, channel_id, channel_data);
    channel_obj.update(channel_data);

    return channel_obj;
}

Channel Chat::get_channel(String channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to get channel, channel_id is empty");
    }

    auto future_response = get_channel_metadata_async(channel_id);
    future_response.wait();
    pubnub_res Res = future_response.get();
    if(Res != PNR_OK)
    {
        throw std::invalid_argument("Failed to get response from server");
    }

    String channel_response = pubnub_get(ctx_pub);

    json response_json = json::parse(channel_response);

    if(response_json.is_null())
    {
        return Channel();
    }

    String channel_data_string = static_cast<Pubnub::String>(response_json["Data"]);
    Channel channel_obj;
    channel_obj.init_from_json(this, channel_id, channel_data_string);

    return channel_obj;
}

std::vector<Channel> Chat::get_channels(String include, int limit, String start, String end)
{
    auto future_response = get_all_channels_metadata_async(include, limit, start, end);
    future_response.wait();
    pubnub_res Res = future_response.get();
    if(Res != PNR_OK)
    {
        throw std::invalid_argument("Failed to get response from server");
    }

    String channels_response = pubnub_get(ctx_pub);

    json response_json = json::parse(channels_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channels, response is incorrect");
    }

    json channel_data_array_json = response_json["Data"];
    std::vector<Channel> Channels;
   
   for (auto& element : channel_data_array_json)
   {
        Channel channel_obj;
        channel_obj.init_from_json(this, String(element["id"]), String(element));
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

    pubnub_remove_channelmetadata(ctx_pub, channel_id);
}

User Chat::create_user(String user_id, ChatUserData user_data)
{
    //Make sure user with this ID doesn't exist
    get_user(user_id);

    User created_user;
    created_user.init(this, user_id, user_data);
    pubnub_set_uuidmetadata(ctx_pub, user_id, NULL, created_user.user_data_to_json(user_id, user_data));

    return created_user;
}

User Chat::get_user(String user_id)
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to get user, user_id is empty");
    }

    auto future_response = get_uuid_metadata_async(user_id);
    future_response.wait();
    pubnub_res Res = future_response.get();
    if(Res != PNR_OK)
    {
        throw std::runtime_error("Failed to get user, incorrect response from server");
    }

    String channel_response = pubnub_get(ctx_pub);

    json response_json = json::parse(channel_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("Failed to get user, response json can't be parsed");
    }

    String user_data_string = static_cast<Pubnub::String>(response_json["Data"]);
    User user_obj;
    user_obj.init_from_json(this, user_id, user_data_string);

    return user_obj;
}

std::vector<User> Chat::get_users(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end)
{
    auto future_response = get_all_uuid_metadata_async(include, limit, start, end);
    future_response.wait();
    pubnub_res Res = future_response.get();
    if(Res != PNR_OK)
    {
        throw std::invalid_argument("Failed to get response from server");
    }

    String users_response = pubnub_get(ctx_pub);

    json response_json = json::parse(users_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channels, response is incorrect");
    }

    json user_data_array_json = response_json["Data"];
    std::vector<User> users;
   
   for (auto& element : user_data_array_json)
   {
        User user_obj;
        user_obj.init_from_json(this, String(element["id"]), String(element));
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

    User user_obj;
    user_obj.init(this, user_id, user_data);
    user_obj.update(user_data);

    return user_obj;
}

void Chat::delete_user(String user_id)
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to delete channel, channel_id is empty");
    }

    pubnub_remove_uuidmetadata(ctx_pub, user_id);
}

void Chat::set_restrictions(String in_user_id, String in_channel_id, bool ban_user, bool mute_user, String reason)
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
	if(!ban_user && !mute_user)
	{
		String remove_member_string = String("[{\"uuid\": {\"id\": \"") + in_user_id + String("\"}}]");
        pubnub_remove_members(ctx_pub, restrictions_channel.c_str(), NULL, remove_member_string.c_str());
		String event_payload_string = String("{\"channelId\": \"") + restrictions_channel + String("\", \"restriction\": \"lifted\", \"reason\": \"") + reason + String("\"}");
        emit_chat_event(pubnub_chat_event_type::PCET_MODERATION, in_user_id, event_payload_string);
		return;
	}

	//Ban or mute the user
	String params_string = String("{\"ban\": ") + bool_to_string(ban_user) + String(", \"mute\": ") + bool_to_string(mute_user) + String(", \"reason\": \"") + reason + String("\"}");
	String set_members_string = String("[{\"uuid\": {\"id\": \"") + user_id + String("\"}, \"custom\": ") + params_string + String("}]");
    pubnub_set_members(ctx_pub, restrictions_channel.c_str(), NULL, set_members_string.c_str());
    String restriction_text;
    ban_user ? restriction_text = "banned" : "muted";
	String event_payload_string = String("{\"channelId\": \"") + restrictions_channel + String("\", \"restriction\": \"lifted") + restriction_text + String("\", \"reason\": \"") + reason + String("\"}");
    emit_chat_event(pubnub_chat_event_type::PCET_MODERATION, in_user_id, event_payload_string);
}

PubnubRestrictionsData Chat::get_user_restrictions(Pubnub::String in_user_id, Pubnub::String in_channel_id, int limit, String start, String end)
{
    auto future_response = get_memberships_async(in_user_id, "totalCount,custom", limit, start, end);
    future_response.wait();
    pubnub_res Res = future_response.get();

    if(Res != PNR_OK)
    {
        throw std::invalid_argument("Failed to get response from server");
    }

    String get_restrictions_response = pubnub_get(ctx_pub);

    json response_json = json::parse(get_restrictions_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get user restrictions, response is incorrect");
    }

    json response_data_json = response_json["data"];
    String full_channel_id = internal_moderation_prefix + in_channel_id;
    PubnubRestrictionsData FinalRestrictionsData;

   for (auto& element : response_data_json)
   {
        //Find restrictions data for requested channel
        if(String(element["channel"]["id"]) == full_channel_id)
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

PubnubRestrictionsData Chat::get_channel_restrictions(Pubnub::String in_user_id, Pubnub::String in_channel_id, int limit, String start, String end)
{
    String full_channel_id = internal_moderation_prefix + in_channel_id;
    auto future_response = get_channel_members_async(full_channel_id, "totalCount,custom", limit, start, end);
    future_response.wait();
    pubnub_res Res = future_response.get();

    if(Res != PNR_OK)
    {
        throw std::invalid_argument("Failed to get response from server");
    }

    String get_restrictions_response = pubnub_get(ctx_pub);

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
    auto future_response = where_now_async(user_id);
    future_response.wait();
    pubnub_res Res = future_response.get();

    if(Res != PNR_OK)
    {
        throw std::invalid_argument("Failed to get response from server");
    }

    String where_now_response = pubnub_get(ctx_pub);

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
    auto future_response = here_now_async(channel_id);
    future_response.wait();
    pubnub_res Res = future_response.get();

    if(Res != PNR_OK)
    {
        throw std::invalid_argument("Failed to get response from server");
    }

    String here_now_response = pubnub_get(ctx_pub);

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
    pubnub_subscribe(ctx_sub, channel_id, NULL);
}

void Chat::unsubscribe_from_channel(String channel_id)
{
    pubnub_leave(ctx_pub, channel_id, NULL);
    pubnub_cancel(ctx_sub);
}

std::future<pubnub_res> Chat::get_channel_metadata_async(const char *channel_id)
{
    return std::async(std::launch::async, [=](){
        pubnub_get_channelmetadata(ctx_pub, NULL, channel_id);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}

std::future<pubnub_res> Chat::get_all_channels_metadata_async(const char *include, int limit, const char *start, const char *end)
{
    return std::async(std::launch::async, [=](){
        pubnub_getall_channelmetadata(ctx_pub, include, limit, start, end, pubnub_tribool::pbccFalse);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}

std::future<pubnub_res> Chat::get_uuid_metadata_async(const char *user_id)
{
    return std::async(std::launch::async, [=](){
        pubnub_get_uuidmetadata(ctx_pub, NULL, user_id);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}

std::future<pubnub_res> Chat::get_all_uuid_metadata_async(const char *include, int limit, const char *start, const char *end)
{
    return std::async(std::launch::async, [=](){
        pubnub_getall_uuidmetadata(ctx_pub, include, limit, start, end, pubnub_tribool::pbccFalse);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}

std::future<pubnub_res> Chat::where_now_async(const char *user_id)
{
        return std::async(std::launch::async, [=](){
        pubnub_where_now(ctx_pub, user_id);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}

std::future<pubnub_res> Chat::here_now_async(const char *channel_id)
{
        return std::async(std::launch::async, [=](){
        pubnub_here_now(ctx_pub, channel_id, NULL);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}

std::future<pubnub_res> Chat::get_memberships_async(const char* user_id, const char* include, int limit, const char* start, const char* end)
{
        return std::async(std::launch::async, [=](){
        pubnub_get_memberships(ctx_pub, user_id, "totalCount,customFields", limit, start, end, pubnub_tribool::pbccFalse);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}

std::future<pubnub_res> Chat::get_channel_members_async(const char* channel_id, const char* include, int limit, const char* start, const char* end)
{
        return std::async(std::launch::async, [=](){
        pubnub_get_members(ctx_pub, channel_id, "totalCount,customFields", limit, start, end, pubnub_tribool::pbccFalse);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}


void Chat::emit_chat_event(pubnub_chat_event_type chat_event_type, String channel_id, String payload)
{
    //Payload is in form of Json: {"param1": "param1value", "param2": "param2value" ... }. So in order to get just parameters, we remove first and last curl bracket
	String payload_parameters = payload;
    payload_parameters.erase(0, 1);
	payload_parameters.erase(payload_parameters.length() - 1);
	String event_message = String("{") + payload_parameters + String(", \"type\": \"") + get_string_from_event_type(chat_event_type) = String("\"}");
    pubnub_publish(ctx_pub, channel_id, event_message);
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
