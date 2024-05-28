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

Channel* Chat::create_public_conversation(String channel_id, ChatChannelData channel_data)
{
    if(channel_id != NULL && *channel_id == 0)
    {
        throw std::invalid_argument("Failed to create public conversation, channel_id is empty");
    }

    Channel* channel_ptr = new Channel;
    channel_ptr->init(this, channel_id, channel_data);

    return channel_ptr;
}

Channel* Chat::update_channel(String channel_id, ChatChannelData channel_data)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to update channel, channel_id is empty");
    }

    Channel* channel_ptr = new Channel;
    channel_ptr->init(this, channel_id, channel_data);

    return channel_ptr;
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

    String channel_data_string = response_json["Data"];
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
        throw std::runtime_error("can't get channels, response is incorrect") ;
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

void Chat::set_restrictions(String in_user_id, String in_channel_id, bool ban_user, bool mute_user, String reason)
{
/*
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
*/
}


User Chat::create_user(String user_id, ChatUserData user_data)
{
    //Make sure user with this ID doesn't exist
    get_user(user_id);

    User created_user;
    created_user.init(this, user_id, user_data);

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

    String user_data_string = response_json["Data"];
    User user_obj;
    user_obj.init_from_json(this, user_id, user_data_string);

    return user_obj;
}


Message Chat::get_message(String MessageTest)
{
    Message new_message;
    new_message.message_id = MessageTest;
    //new_message.message_text = MessageTest;
    return new_message;
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

void Chat::emit_chat_event(pubnub_chat_event_type chat_event_type, String channel_id, String payload)
{
    /*
    //Payload is in form of Json: {"param1": "param1value", "param2": "param2value" ... }. So in order to get just parameters, we remove first and last curl bracket
	String payload_parameters = payload;
    payload_parameters.erase(0, 1);
	payload_parameters.erase(payload_parameters.length() - 1);
	String event_message = String("{") + payload_parameters + String(", \"type\": \"") + get_string_from_event_type(chat_event_type) = String("\"}");
    pubnub_publish(ctx_pub, channel_id, event_message);
    */
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
