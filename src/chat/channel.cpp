#include "chat/channel.hpp"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include "infra/pubnub.hpp"
#include "nlohmann/json.hpp"
#include "chat.hpp"
#include "chat/chat_helpers.hpp"
#include "infra/timer.hpp"

extern "C" {
#include "core/pubnub_objects_api.h"
#include "core/pubnub_ntf_sync.h"
#include "core/pubnub_pubsubapi.h"
#include "core/pubnub_memory_block.h"  // required to include pubnub_fetch_history.h without errors
#include "core/pubnub_fetch_history.h"
}


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

Pubnub::Channel Channel::update(ChatChannelData in_additional_channel_data)
{
    // TODO: transactional update
    channel_data = in_additional_channel_data;

    this->chat_obj
        .get_pubnub_context()
        .set_channel_metadata(channel_id, channel_data_to_json(channel_id, channel_data));

    return *this;
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

std::vector<String> Channel::connect_and_get_messages()
{
    //TODO: this function in current state will return all published messages, not only those that we
    //consider to be chat sdk messages, but also technical ones.
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
    String set_object_string = create_set_memberships_object(channel_id, additional_params);

    String user_id = this->chat_obj.get_pubnub_context().get_user_id();
    this->chat_obj
        .get_pubnub_context()
        .set_memberships(user_id, set_object_string);

    connect(message_callback);
}

void Channel::join(CallbackStringFunction string_callback, Pubnub::String additional_params)
{
    auto callback = [string_callback](Pubnub::Message message) {
        string_callback(message.to_string().c_str());
    };

    join(callback, additional_params);
}

std::vector<String> Channel::join_and_get_messages(Pubnub::String additional_params)
{
    String include_string = "totalCount,customFields,channelFields,customChannelFields";
    String set_object_string = create_set_memberships_object(channel_id, additional_params);

    String user_id = this->chat_obj.get_pubnub_context().get_user_id();

    this->chat_obj
        .get_pubnub_context()
        .set_memberships(user_id, set_object_string);

    return connect_and_get_messages();
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

    String in_timetoken = timetoken;
    auto start_timetoken_int = std::stoull(timetoken.to_std_string()) + 1;
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
        Membership membership_obj(chat_obj, *this, String(element["uuid"].dump()));
        memberships.push_back(membership_obj);
    }

    return memberships;
}

Pubnub::Membership Channel::invite(Pubnub::User user)
{
    /* disabled for testing
    if(channel_data.type == String("public"))
    {
        throw std::runtime_error("Channel invites are not supported in Public chats");
    }*/

    //TODO:: check here if user already is on that channel. Requires C-Core filtering

    String include_string = "totalCount,customFields,channelFields,customChannelFields";
    String set_memeberships_obj = create_set_memberships_object(channel_id, "");
    String memberships_response = chat_obj.get_pubnub_context().set_memberships(user.get_user_id(), set_memeberships_obj, include_string);
    
    json memberships_response_json = json::parse(memberships_response);

    String channel_data_string = memberships_response_json["data"][0].dump();

    String event_payload = "{\"channelType\": \"" + channel_data.type + "\", \"channelId\": \"" + channel_id + "\"}";
    chat_obj.emit_chat_event(pubnub_chat_event_type::PCET_INVITE, user.get_user_id(), event_payload);
    
    return Membership(chat_obj, user, channel_data_string);

}

std::vector<Pubnub::Membership> Channel::invite_multiple(std::vector<Pubnub::User> users)
{
    /* disabled for testing
    if(channel_data.type == String("public"))
    {
        throw std::runtime_error("Channel invites are not supported in Public chats");
    }*/

    //TODO:: check here if users already are on that channel. Requires C-Core filtering
    std::vector<String> filtered_users_ids;

    for(auto &user : users)
    {
        filtered_users_ids.push_back(user.get_user_id());
    }

    String include_string = "totalCount,customFields,channelFields,customChannelFields";
    String set_memebers_obj = create_set_members_object(filtered_users_ids, "");
    String set_members_response = chat_obj.get_pubnub_context().set_members(this->channel_id, set_memebers_obj, include_string);
    
    std::vector<Pubnub::Membership> invitees_memberships;

    json memberships_response_json = json::parse(set_members_response);
    json memberships_data_array = memberships_response_json["data"];

    String test = memberships_data_array.dump();
    

    for (json::iterator single_data_json = memberships_data_array.begin(); single_data_json != memberships_data_array.end(); ++single_data_json) 
    {
        Pubnub::String membership_data = single_data_json.value().dump();
        Pubnub::Membership membership_obj(chat_obj, *this, membership_data);
        invitees_memberships.push_back(membership_obj);

        String event_payload = "{\"channelType\": \"" + channel_data.type + "\", \"channelId\": \"" + channel_id + "\"}";
        chat_obj.emit_chat_event(pubnub_chat_event_type::PCET_INVITE, membership_obj.get_user_id(), event_payload);
    }

    return invitees_memberships;

}

void Channel::start_typing()
{
    /* disabled for testing
    if(channel_data.type == String("public"))
    {
        throw std::runtime_error("Typing indicators are not supported in Public chats");
    }*/
    if(typing_sent) return;

    typing_sent = true;
    typing_sent_timer = Timer();
    typing_sent_timer.start(TYPING_TIMEOUT - 1000, [=](){
        typing_sent = false;
    });
    
    typing_sent = true;
    chat_obj.emit_chat_event(pubnub_chat_event_type::PCET_TYPING, channel_id, "{\"value\": true}");

}

void Channel::stop_typing()
{
    /* disabled for testing
    if(channel_data.type == String("public"))
    {
        throw std::runtime_error("Typing indicators are not supported in Public chats");
    }*/
    typing_sent_timer.stop();

    if(!typing_sent) return;

    typing_sent = false;
    chat_obj.emit_chat_event(pubnub_chat_event_type::PCET_TYPING, channel_id, "{\"value\": false}");
}

void Channel::stream_updates(std::function<void(Channel)> channel_callback)
{
    std::vector<Pubnub::Channel> channels;
    channels.push_back(*this);
    stream_updates_on(channels, channel_callback);
}

void Channel::stream_updates_on(std::vector<Pubnub::Channel> channels, std::function<void(Channel)> channel_callback)
{
    if(channels.empty())
    {
        throw std::invalid_argument("Cannot stream channel updates on an empty list");
    }

    for(auto channel : channels)
    {
        chat_obj.get_pubnub_context().register_channel_callback(channel.channel_id, channel_callback);
        chat_obj.get_pubnub_context().subscribe_to_channel(channel.channel_id);
    }
}

void Channel::stream_presence(std::function<void(std::vector<Pubnub::String>)> presence_callback)
{
    //Send callback with currently present users
    std::vector<Pubnub::String> current_users = who_is_present();
    presence_callback(current_users);

    String presence_channel = channel_id + "-pnpres";
    chat_obj.get_pubnub_context().subscribe_to_channel(presence_channel);
    chat_obj.get_pubnub_context().register_channel_presence_callback(channel_id, presence_callback);
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

    if(channel_data_json.contains("name") && !channel_data_json["name"].is_null())
    {
        channel_data.channel_name = channel_data_json["name"].dump();
    }
    if(channel_data_json.contains("description") && !channel_data_json["description"].is_null())
    {
        channel_data.description = channel_data_json["description"].dump();
    }
    if(channel_data_json.contains("custom") && !channel_data_json["custom"].is_null())
    {
        channel_data.custom_data_json = channel_data_json["custom"].dump();
    }
    if(channel_data_json.contains("updated") && !channel_data_json["updated"].is_null())
    {
        channel_data.updated = channel_data_json["updated"].dump();
    }
    if(channel_data_json.contains("status") && !channel_data_json["status"].is_null())
    {
        channel_data.status = channel_data_json["status"].dump();
    }
    if(channel_data_json.contains("type") && !channel_data_json["type"].is_null())
    {
        channel_data.type = channel_data_json["type"].dump();
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

    if(channel_data_json.contains("channel") )
    {
        auto dumped = channel_data_json["channel"]["id"].dump();

        // removal of quotes
        return String(&dumped[1], dumped.size() - 2);
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

