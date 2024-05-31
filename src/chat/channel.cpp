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

void Channel::init(Chat *InChat, String in_channel_id, ChatChannelData in_additional_channel_data)
{
    chat_obj = InChat;
    channel_id = in_channel_id;
    channel_data = in_additional_channel_data;

    //now channel is fully initialized
    is_initialized = true;
}

void Channel::init_from_json(Chat *InChat, String in_channel_id, String channel_data_json)
{
    init(InChat, in_channel_id, channel_data_from_json(channel_data_json));
}

void Channel::update(ChatChannelData in_additional_channel_data)
{
    channel_data = in_additional_channel_data;
    pubnub_set_channelmetadata(get_ctx_pub(), channel_id, NULL, channel_data_to_json(channel_id, channel_data));
}

void Channel::connect()
{
    if(!chat_obj)
    {
        throw std::invalid_argument("Failed to connect to channel, chat_obj is invalid");
    }

    chat_obj->subscribe_to_channel(channel_id);
}

void Channel::disconnect()
{
    if(!chat_obj)
    {
        throw std::invalid_argument("Failed to disconnect from channel, chat_obj is invalid");
    }

    chat_obj->unsubscribe_from_channel(channel_id);
}

void Channel::join(String additional_params)
{
    String include_string = "totalCount,customFields,channelFields,customChannelFields";
    String custom_parameter_string;
    additional_params.empty() ? custom_parameter_string="{}" : custom_parameter_string = additional_params;
    String set_object_string = String("[{\"channel\": {\"id\": \"") + channel_id +  String("\"}, \"custom\": ") + additional_params + String("}]");
    pubnub_set_memberships(get_ctx_pub(), pubnub_user_id_get(get_ctx_pub()), include_string, set_object_string);

    connect();
}

void Channel::leave()
{
    String remove_object_string = String("[{\"channel\": {\"id\": \"") + channel_id + String("\"}}]");
    pubnub_remove_memberships(get_ctx_pub(), pubnub_user_id_get(get_ctx_pub()), NULL, remove_object_string);

	disconnect();
}

void Channel::delete_channel()
{
    if(!chat_obj)
    {
        throw std::invalid_argument("Failed to delete channel, chat_obj is invalid");
    }

    chat_obj->delete_channel(channel_id);
}

void Channel::set_restrictions(String in_user_id, bool ban_user, bool mute_user, String reason)
{
    if(!chat_obj)
    {
        throw std::invalid_argument("Failed to set restrictions, chat_obj is invalid");
    }

    chat_obj->set_restrictions(in_user_id, channel_id, ban_user, mute_user, reason);
}

void Channel::send_text(String message, pubnub_chat_message_type message_type, String meta_data)
{
    if(message.empty())
    {
        throw std::invalid_argument("Failed to send text, message is empty");
    }

    pubnub_publish(get_ctx_pub(), channel_id, chat_message_to_publish_string(message, message_type));

    pubnub_res publish_response =  pubnub_await(get_ctx_pub());

    if(publish_response != PNR_OK)
    {
        throw std::runtime_error("Failed to publish message");
    }

}

Pubnub::String Channel::get_channel_id()
{
    return channel_id;
}

ChatChannelData Channel::get_channel_data(){
    return channel_data;
}

ChatChannelData Channel::channel_data_from_json(String data_json_string)
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

String Channel::channel_data_to_json(String in_channel_id, ChatChannelData in_channel_data)
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

String Channel::chat_message_to_publish_string(String message, pubnub_chat_message_type message_type)
{
    json message_json;
	
	String message_type_string;
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
