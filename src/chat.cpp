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

void Chat::init(std::string in_publish_key, std::string in_subscribe_key, std::string in_user_id)
{
    init(in_publish_key.c_str(), in_subscribe_key.c_str(), in_user_id.c_str());
}

void Chat::init(const char* in_publish_key, const char* in_subscribe_key, const char* in_user_id)
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


void Chat::publish_message(std::string channel, std::string message)
{
    publish_message(channel.c_str(), message.c_str());
}

void Chat::publish_message(const char* channel, const char*message)
{
    printf("publish message");

    pubnub_publish(ctx_pub, channel, message);

    pubnub_await(ctx_pub);

    std::cout << "Message published\n";
}

Pubnub::Channel* Chat::create_public_conversation(std::string channel_id, ChatChannelData channel_data)
{
    return create_public_conversation(channel_id.c_str(), ChatChannelDataChar(channel_data));
}

Pubnub::Channel* Chat::create_public_conversation(const char* channel_id, ChatChannelDataChar channel_data)
{
    if(channel_id != NULL && *channel_id == 0)
    {
        throw std::invalid_argument("Failed to create public conversation, channel_id is empty");
    }

    Channel* channel_ptr = new Channel;
    channel_ptr->init(this, channel_id, channel_data);

    return channel_ptr;
}

Pubnub::Channel* Chat::update_channel(std::string channel_id, ChatChannelData channel_data)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to update channel, channel_id is empty");
    }

    Channel* channel_ptr = new Channel;
    channel_ptr->init(this, channel_id, channel_data);

    return channel_ptr;
}

Pubnub::Channel* Chat::update_channel(const char* channel_id, ChatChannelDataChar channel_data)
{
    std::string channel_id_string = channel_id;

    return update_channel(channel_id_string, ChatChannelData(channel_data));
}

Channel Pubnub::Chat::get_channel(std::string channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to get channel, channel_id is empty");
    }

    auto future_response = get_channel_metadata_async(channel_id.c_str());
    future_response.wait();
    pubnub_res Res = future_response.get();
    if(Res != PNR_OK)
    {
        throw std::invalid_argument("Failed to get response from server");
    }

    std::string channel_response = pubnub_get(ctx_pub);

    json response_json = json::parse(channel_response);

    if(response_json.is_null())
    {
        return Channel();
    }

    std::string channel_data_string = response_json["Data"];
    Channel channel_obj;
    channel_obj.init_from_json(this, channel_id, channel_data_string);

    return channel_obj;
}

Channel Chat::get_channel(const char *channel_id)
{
    std::string channel_id_string = channel_id;
    return get_channel(channel_id_string);
}

std::vector<Channel> Chat::get_channels(std::string include, int limit, std::string start, std::string end)
{
    auto future_response = get_all_channels_metadata_async(include.c_str(), limit, start.c_str(), end.c_str());
    future_response.wait();
    pubnub_res Res = future_response.get();
    if(Res != PNR_OK)
    {
        throw std::invalid_argument("Failed to get response from server");
    }

    std::string channels_response = pubnub_get(ctx_pub);

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
        channel_obj.init_from_json(this, element["id"], element);
        Channels.push_back(channel_obj);
   }

    return Channels;
}

std::vector<Channel> Chat::get_channels(const char* include, int limit, const char* start, const char* end)
{
    std::string include_string = include;
    std::string start_string = start;
    std::string end_string = end;
    return get_channels(include_string, limit, start_string, end_string);
}

void Pubnub::Chat::delete_channel(std::string channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to delete channel, channel_id is empty");
    }

    pubnub_remove_channelmetadata(ctx_pub, channel_id.c_str());
}

void Pubnub::Chat::delete_channel(const char *channel_id)
{
    std::string channel_id_string = channel_id;
    delete_channel(channel_id_string);
}

void Pubnub::Chat::set_restrictions(std::string in_user_id, std::string in_channel_id, bool ban_user, bool mute_user, std::string reason)
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
	std::string restrictions_channel = internal_moderation_prefix + in_channel_id;

	//Lift restrictions
	if(!ban_user && !mute_user)
	{
		std::string remove_member_string = std::string("[{\"uuid\": {\"id\": \"") + in_user_id + std::string("\"}}]");
        pubnub_remove_members(ctx_pub, restrictions_channel.c_str(), NULL, remove_member_string.c_str());
		std::string event_payload_string = std::string("{\"channelId\": \"") + restrictions_channel + std::string("\", \"restriction\": \"lifted\", \"reason\": \"") + reason + std::string("\"}");
        emit_chat_event(pubnub_chat_event_type::PCET_MODERATION, in_user_id, event_payload_string);
		return;
	}

	//Ban or mute the user
	std::string params_string = std::string("{\"ban\": ") + bool_to_string(ban_user) + std::string(", \"mute\": ") + bool_to_string(mute_user) + std::string(", \"reason\": \"") + reason + std::string("\"}");
	std::string set_members_string = std::string("[{\"uuid\": {\"id\": \"") + user_id + std::string("\"}, \"custom\": ") + params_string + std::string("}]");
    pubnub_set_members(ctx_pub, restrictions_channel.c_str(), NULL, set_members_string.c_str());
    std::string restriction_text;
    ban_user ? restriction_text = "banned" : "muted";
	std::string event_payload_string = std::string("{\"channelId\": \"") + restrictions_channel + std::string("\", \"restriction\": \"lifted") + restriction_text + std::string("\", \"reason\": \"") + reason + std::string("\"}");
    emit_chat_event(pubnub_chat_event_type::PCET_MODERATION, in_user_id, event_payload_string);
}

void Pubnub::Chat::set_restrictions(const char *in_user_id, const char* in_channel_id, bool ban_user, bool mute_user, const char *reason)
{
    std::string in_user_id_string = in_user_id;
    std::string in_channel_id_string = in_channel_id;
    std::string reason_string = reason;
    set_restrictions(in_user_id_string, in_channel_id_string, ban_user, mute_user, reason_string);
}

Message Chat::get_message(String MessageTest)
{
    Message new_message;
    new_message.message_id = MessageTest;
    //new_message.message_text = MessageTest;
    return new_message;
}

void Pubnub::Chat::subscribe_to_channel(const char *channel_id)
{
    pubnub_subscribe(ctx_sub, channel_id, NULL);
}

void Pubnub::Chat::unsubscribe_from_channel(const char *channel_id)
{
    pubnub_leave(ctx_pub, channel_id, NULL);
    pubnub_cancel(ctx_sub);
}

std::future<pubnub_res> Pubnub::Chat::get_channel_metadata_async(const char *channel_id)
{
    return std::async(std::launch::async, [=](){
        pubnub_get_channelmetadata(ctx_pub, NULL, channel_id);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}

std::future<pubnub_res> Pubnub::Chat::get_all_channels_metadata_async(const char *include, int limit, const char *start, const char *end)
{
    return std::async(std::launch::async, [=](){
        pubnub_getall_channelmetadata(ctx_pub, include, limit, start, end, pubnub_tribool::pbccFalse);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}

void Pubnub::Chat::emit_chat_event(pubnub_chat_event_type chat_event_type, std::string channel_id, std::string payload)
{
    //Payload is in form of Json: {"param1": "param1value", "param2": "param2value" ... }. So in order to get just parameters, we remove first and last curl bracket
	std::string payload_parameters = payload;
    payload_parameters.erase(0, 1);
	payload_parameters.erase(payload_parameters.size() - 1);
	std::string event_message = std::string("{") + payload_parameters + std::string(", \"type\": \"") + get_string_from_event_type(chat_event_type) = std::string("\"}");
    pubnub_publish(ctx_pub, channel_id.c_str(), event_message.c_str());
}

std::string Pubnub::Chat::get_string_from_event_type(pubnub_chat_event_type chat_event_type)
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
