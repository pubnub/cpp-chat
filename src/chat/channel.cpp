#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include "infra/pubnub.hpp"
#include "nlohmann/json.hpp"
#include "chat.hpp"

extern "C" {
#include "core/pubnub_objects_api.h"
#include "core/pubnub_ntf_sync.h"
#include "core/pubnub_pubsubapi.h"
#include "core/pubnub_memory_block.h"  // required to include pubnub_fetch_history.h without errors
#include "core/pubnub_fetch_history.h"
}

#include "chat/channel.hpp"
#include "chat/message.hpp"
#include "chat/membership.hpp"


using namespace Pubnub;
using json = nlohmann::json;

Channel::Channel(Chat& InChat, String in_channel_id, ChatChannelData in_additional_channel_data) :
    chat_obj(InChat),
    channel_id(in_channel_id),
    channel_data(in_additional_channel_data)
{}

Channel::Channel(Chat& InChat, String in_channel_id, String channel_data_json) :
    Channel(InChat, in_channel_id, channel_data_from_json(channel_data_json)) {}

Channel::Channel(Pubnub::Chat& InChat, String channel_data_json) :
    Channel(InChat, channel_id_from_json(channel_data_json), channel_data_from_json(channel_data_json)) {}

void Channel::update(ChatChannelData in_additional_channel_data)
{
    // TODO: transactional update
    channel_data = in_additional_channel_data;

    this->chat_obj
        .get_pubnub_context()
        .set_channel_metadata(channel_id, channel_data_to_json(channel_id, channel_data));
}

void Channel::connect(std::function<void(Message)> message_callback) {
    this->chat_obj.subscribe_to_channel(channel_id);
    this->chat_obj.get_pubnub_context().register_message_callback(channel_id, message_callback);
}

void Channel::connect(CallbackStringFunction string_callback)
{
    auto callback = [string_callback](Pubnub::Message message) {
        auto string = message.to_string();
        string_callback(string.c_str());
    };

    this->connect(callback);
}

std::vector<Message> Channel::connect_and_get_messages()
{
    return this->chat_obj.subscribe_to_channel_and_get_last_messages(this->channel_id);
}

void Channel::disconnect()
{
    this->chat_obj.unsubscribe_from_channel(channel_id);
    this->chat_obj
        .get_pubnub_context()
        .remove_message_callback(channel_id);
}

void Channel::join(std::function<void(Message)> message_callback, Pubnub::String additional_params)
{
    String include_string = "totalCount,customFields,channelFields,customChannelFields";
    String custom_parameter_string;
    additional_params.empty() ? custom_parameter_string="{}" : custom_parameter_string = additional_params;
    String set_object_string = String("[{\"channel\": {\"id\": \"") + channel_id +  String("\"}, \"custom\": ") + custom_parameter_string + String("}]");

    this->chat_obj
        .get_pubnub_context()
        .set_memberships(channel_id, set_object_string);

    connect(message_callback);
}

void Channel::join(CallbackStringFunction string_callback, Pubnub::String additional_params)
{
    auto callback = [string_callback](Pubnub::Message message) {
        string_callback(message.to_string().c_str());
    };

    join(callback, additional_params);
}

void Channel::leave()
{
    String remove_object_string = String("[{\"channel\": {\"id\": \"") + channel_id + String("\"}}]");

    auto& pubnub_context = this->chat_obj.get_pubnub_context();
    pubnub_context.remove_memberships(pubnub_context.get_user_id(), remove_object_string);

	disconnect();
}

void Channel::delete_channel()
{
    this->chat_obj.delete_channel(channel_id);
}

void Channel::set_restrictions(String in_user_id, PubnubRestrictionsData restrictions)
{
    this->chat_obj.set_restrictions(in_user_id, channel_id, restrictions);
}

void Channel::send_text(String message, pubnub_chat_message_type message_type, String meta_data)
{
    this->chat_obj
        .get_pubnub_context()
        .publish(channel_id, chat_message_to_publish_string(message, message_type));
}

std::vector<Pubnub::String> Channel::who_is_present()
{
    return this->chat_obj.who_is_present(channel_id);
}

bool Channel::is_present(Pubnub::String user_id)
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to get is present, channel_id is empty");
    }

    return this->chat_obj.is_present(user_id, channel_id);
}

std::vector<Message> Channel::get_history(Pubnub::String start_timetoken, Pubnub::String end_timetoken, int count)
{
    String fetch_history_response = this->chat_obj
        .get_pubnub_context()
        .fetch_history(channel_id, start_timetoken, end_timetoken, count);

    json response_json = json::parse(fetch_history_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get history, response is incorrect");
    }

    json messages_array_json = response_json["channels"][channel_id];


    std::vector<Message> messages;

    for (auto& element : messages_array_json)
    {
        Message message_obj(chat_obj, channel_id, element.dump());
        messages.push_back(message_obj);
    }

    return messages;
}

Message Channel::get_message(Pubnub::String timetoken)
{
    int64_t start_timetoken_int = std::stoll(timetoken) + 1;
    String start_timetoken = std::to_string(start_timetoken_int);
    std::vector<Message> messages = get_history(start_timetoken, timetoken, 1);
    if(messages.size() == 0)
    {
        throw std::runtime_error("can't get message, there is no message with this timestamp");
    }

    return messages[0];
}

PubnubRestrictionsData Channel::get_user_restrictions(Pubnub::String in_user_id, Pubnub::String in_channel_id, int limit, String start, String end)
{
    String get_restrictions_response = chat_obj.get_pubnub_context().get_memberships(in_user_id, "totalCount,custom", limit, start, end);

    json response_json = json::parse(get_restrictions_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get user restrictions, response is incorrect");
    }

    json response_data_json = response_json["data"];
    String full_channel_id = chat_obj.internal_moderation_prefix + in_channel_id;
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

std::vector<Pubnub::Membership> Channel::get_members(int limit, Pubnub::String start_timetoken, Pubnub::String end_timetoken)
{
    String include_string = "totalCount,customFields,channelFields,customChannelFields";
    String get_channel_members_response = chat_obj.get_pubnub_context().get_channel_members(channel_id, include_string, limit, start_timetoken, end_timetoken);

    json response_json = json::parse(get_channel_members_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get members, response is incorrect");
    }

    json users_array_json = response_json["data"];


    std::vector<Membership> memberships;

    for (auto& element : users_array_json)
    {
        Membership membership_obj(chat_obj, *this, String(element["uuid"]));
        memberships.push_back(membership_obj);
    }

    return memberships;
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

String Channel::channel_id_from_json(String data_json_string)
{
    json channel_data_json = json::parse(data_json_string);;

    if(channel_data_json.is_null())
    {
        return "";
    }

    if(channel_data_json.contains("id") )
    {
        return channel_data_json["id"].dump();
    }

    return "";
}

String Channel::channel_data_to_json(String in_channel_id, ChatChannelData in_channel_data)
{
    json channel_data_json;

    channel_data_json["id"] = in_channel_id.c_str();
    if(!in_channel_data.channel_name.empty())
    {
        channel_data_json["name"] = in_channel_data.channel_name.c_str();
    }
    if(!in_channel_data.description.empty())
    {
        channel_data_json["description"] = in_channel_data.description.c_str();
    }
    if(!in_channel_data.custom_data_json.empty())
    {
        json custom_json = json::parse(in_channel_data.custom_data_json);
        channel_data_json["custom"] = custom_json;
    }
    if(!in_channel_data.updated.empty())
    {
        channel_data_json["updated"] = in_channel_data.updated.c_str();
    }
    if(!in_channel_data.status.empty())
    {
        channel_data_json["status"] = in_channel_data.status.c_str();
    }
    if(!in_channel_data.type.empty())
    {
        channel_data_json["type"] = in_channel_data.type.c_str();
    }

    return channel_data_json.dump();
}

String Channel::chat_message_to_publish_string(String message, pubnub_chat_message_type message_type)
{
    json message_json;
	
	message_json["type"] = Pubnub::chat_message_type_to_string(message_type).c_str();
    message_json["text"] = message.c_str();

	//Convert constructed Json to FString
	return message_json.dump();
}

