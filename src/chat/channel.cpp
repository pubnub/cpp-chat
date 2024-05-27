#include <iostream>
#include <stdexcept>
#include "nlohmann/json.hpp"
#include "chat.hpp"

extern "C" {
#include "core/pubnub_objects_api.h"
#include "core/pubnub_ntf_sync.h"
#include "core/pubnub_pubsubapi.h"
}

#include "chat/channel.hpp"


using namespace Pubnub;
using json = nlohmann::json;

void Channel::init(Pubnub::Chat *InChat, std::string in_channel_id, ChatChannelData in_additional_channel_data)
{
    chat_obj = InChat;
    channel_id = in_channel_id;
    channel_data = in_additional_channel_data;

    pubnub_set_channelmetadata(get_ctx_pub(), channel_id.c_str(), NULL, channel_data_to_json(channel_id, channel_data).c_str());

    //now channel is fully initialized
    is_initialized = true;
}

void Channel::init(Pubnub::Chat *InChat, const char* in_channel_id, ChatChannelDataChar in_additional_channel_data)
{
    std::string channel_id_string = in_channel_id;
    init(InChat, channel_id_string, ChatChannelData(in_additional_channel_data));
}

void Channel::init_from_json(Pubnub::Chat *InChat, std::string in_channel_id, std::string channel_data_json)
{
    init(InChat, in_channel_id, channel_data_from_json(channel_data_json));
}

void Channel::init_from_json(Pubnub::Chat *InChat, const char *in_channel_id, const char *channel_data_json)
{
    std::string channel_id_string = in_channel_id;
    std::string channel_data_json_string = channel_data_json;
    init_from_json(InChat, channel_id_string, channel_data_json_string);
}

void Channel::update(ChatChannelData in_additional_channel_data)
{
    channel_data = in_additional_channel_data;
    pubnub_set_channelmetadata(get_ctx_pub(), channel_id.c_str(), NULL, channel_data_to_json(channel_id, channel_data).c_str());
}

void Channel::update(ChatChannelDataChar in_additional_channel_data)
{
    update(ChatChannelData(in_additional_channel_data));
}

void Pubnub::Channel::connect()
{
    if(!chat_obj)
    {
        throw std::invalid_argument("Failed to connect to channel, chat_obj is invalid");
    }

    chat_obj->subscribe_to_channel(channel_id.c_str());
}

void Pubnub::Channel::disconnect()
{
    if(!chat_obj)
    {
        throw std::invalid_argument("Failed to disconnect from channel, chat_obj is invalid");
    }

    chat_obj->unsubscribe_from_channel(channel_id.c_str());
}

void Pubnub::Channel::join(std::string additional_params)
{
    std::string include_string = "totalCount,customFields,channelFields,customChannelFields";
    std::string custom_parameter_string;
    additional_params.empty() ? custom_parameter_string="{}" : custom_parameter_string = additional_params;
    std::string set_object_string = std::string("[{\"channel\": {\"id\": \"") + channel_id +  std::string("\"}, \"custom\": ") + additional_params + std::string("}]");
    pubnub_set_memberships(get_ctx_pub(), pubnub_user_id_get(get_ctx_pub()), include_string.c_str(), set_object_string.c_str());

    connect();
}

void Pubnub::Channel::Join(const char *additional_params)
{
    std::string additional_params_string = additional_params;
    join(additional_params_string);
}

void Pubnub::Channel::leave()
{
    std::string remove_object_string = std::string("[{\"channel\": {\"id\": \"") + channel_id + std::string("\"}}]");
    pubnub_remove_memberships(get_ctx_pub(), pubnub_user_id_get(get_ctx_pub()), NULL, remove_object_string.c_str());

	disconnect();
}

void Pubnub::Channel::delete_channel()
{
    if(!chat_obj)
    {
        throw std::invalid_argument("Failed to delete channel, chat_obj is invalid");
    }

    chat_obj->delete_channel(channel_id);
}

void Pubnub::Channel::set_restrictions(std::string in_user_id, bool ban_user, bool mute_user, std::string reason)
{
    if(!chat_obj)
    {
        throw std::invalid_argument("Failed to set restrictions, chat_obj is invalid");
    }

    chat_obj->set_restrictions(in_user_id, channel_id, ban_user, mute_user, reason);
}
void Pubnub::Channel::set_restrictions(const char *in_user_id, bool ban_user, bool mute_user, const char *reason)
{
    std::string in_user_id_string = in_user_id;
    std::string reason_string = reason;
    set_restrictions(in_user_id_string, ban_user, mute_user, reason_string);
}

void Pubnub::Channel::send_text(std::string message, pubnub_chat_message_type message_type, std::string meta_data)
{
    if(message.empty())
    {
        throw std::invalid_argument("Failed to send text, message is empty");
    }

    pubnub_publish(get_ctx_pub(), channel_id.c_str(), chat_message_to_publish_string(message, message_type).c_str());

    pubnub_res publish_response =  pubnub_await(get_ctx_pub());

    if(publish_response != PNR_OK)
    {
        throw std::runtime_error("Failed to publish message");
    }

}

void Pubnub::Channel::send_text(const char *message, pubnub_chat_message_type message_type, const char *meta_data)
{
    std::string message_string = message;
    std::string meta_data_string = meta_data;
    send_text(message_string, message_type, meta_data_string);
}

ChatChannelData Channel::channel_data_from_json(std::string data_json_string)
{
    json channel_data_json = json::parse(data_json_string);;

    if(channel_data_json.is_null())
    {
        return ChatChannelData();
    }

    ChatChannelData channel_data;

    if(channel_data_json.contains("name") )
    {
        channel_data.channel_name = channel_data_json["name"];
    }
    if(channel_data_json.contains("description") )
    {
        channel_data.description = channel_data_json["description"];
    }
    if(channel_data_json.contains("custom") )
    {
        channel_data.custom_data_json = channel_data_json["custom"];
    }
    if(channel_data_json.contains("updated") )
    {
        channel_data.updated = channel_data_json["updated"];
    }
    if(channel_data_json.contains("status") )
    {
        channel_data.status = channel_data_json["status"];
    }
    if(channel_data_json.contains("type") )
    {
        channel_data.type = channel_data_json["type"];
    }

    return channel_data;
}

std::string Channel::channel_data_to_json(std::string in_channel_id, ChatChannelData in_channel_data)
{
    json channel_data_json;

    channel_data_json["id"] = in_channel_id;
    if(in_channel_data.channel_name.empty())
    {
        channel_data_json["name"] = in_channel_data.channel_name;
    }
    if(in_channel_data.description.empty())
    {
        channel_data_json["description"] = in_channel_data.description;
    }
    if(in_channel_data.custom_data_json.empty())
    {
        channel_data_json["custom"] = in_channel_data.custom_data_json;
    }
    if(in_channel_data.updated.empty())
    {
        channel_data_json["updated"] = in_channel_data.updated;
    }
    if(in_channel_data.status != 0 )
    {
        channel_data_json["status"] = in_channel_data.status;
    }
    if(in_channel_data.type.empty())
    {
        channel_data_json["type"] = in_channel_data.type;
    }

    return channel_data_json.dump();
}

std::string Channel::chat_message_to_publish_string(std::string message, pubnub_chat_message_type message_type)
{
    json message_json;
	
	std::string message_type_string;
    //For now there is only one type, but we might want to add more types in the future
	switch (message_type)
	{
	case pubnub_chat_message_type::PCMT_TEXT:
		message_type_string = "text";
		break;
	}
	message_json["type"] = message_type_string;
    message_json["text"] = message;


	//Convert constructed Json to FString
	return message_json.dump();
}


pubnub_t* Channel::get_ctx_pub()
{
    if(!chat_obj)
    //TODO: throw exception, error or sth
    {return nullptr;}
    return chat_obj->get_pubnub_context();
}
