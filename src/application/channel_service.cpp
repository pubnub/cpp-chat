#include "channel_service.hpp"
#include "thread_channel.hpp"
#include "thread_message.hpp"
#include "application/dao/channel_dao.hpp"
#include "chat_service.hpp"
#include "domain/channel_entity.hpp"
#include "domain/json.hpp"
#include "domain/typing.hpp"
#include "user_service.hpp"
#include "membership_service.hpp"
#include "message_service.hpp"
#include "message.hpp"
#include "const_values.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "infra/timer.hpp"
#include "chat_helpers.hpp"
#include "nlohmann/json.hpp"
#include "application/callback_service.hpp"
#include "option.hpp"
#ifdef PN_CHAT_C_ABI
#include <pubnub_helper.h>
#include "domain/parsers.hpp"
#endif // PN_CHAT_C_ABI

using namespace Pubnub;
using json = nlohmann::json;

ChannelService::ChannelService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
    chat_service(chat_service)
{}

Channel ChannelService::create_public_conversation(const String& channel_id, const ChannelDAO& data) const {
    auto new_entity = data.to_entity();
    new_entity.type = "public";
    return create_channel(channel_id, std::move(new_entity));
}

std::tuple<Channel, Membership, std::vector<Membership>> ChannelService::create_direct_conversation(const User& user, const String& channel_id, const ChannelDAO& channel_data, const String& membership_data) const {
    //TODO: channel id should be optional and if it's not provided, we should create hashed channel id
    String final_channel_id = channel_id;

    auto new_entity = channel_data.to_entity();
    new_entity.type = "direct";
    auto created_channel = this->create_channel(final_channel_id, std::move(new_entity));
    String user_id;

    {
        auto pubnub_handle = this->pubnub->lock();

        //TODO: Add filter when it will be supported in C-Core
        String include_string = "custom,channel,totalCount,customChannel";
        user_id = pubnub_handle->get_user_id();
        pubnub_handle->set_memberships(pubnub_handle->get_user_id(), create_set_memberships_object(final_channel_id), include_string);
    }

    auto chat_service_shared = chat_service.lock();

    //TODO: Maybe current user should just be created in chat constructor and stored there all the time?
    User current_user = chat_service_shared->user_service->get_user(user_id);

    Membership host_membership = chat_service_shared->membership_service->create_membership_object(current_user, created_channel);
    Membership invitee_membership = chat_service_shared->membership_service->invite_to_channel(final_channel_id, *created_channel.data, user);

    return std::make_tuple(created_channel, host_membership, std::vector<Membership>{invitee_membership});
}

std::tuple<Channel, Membership, std::vector<Membership>> ChannelService::create_group_conversation(const std::vector<User>& users, const String& channel_id, const ChannelDAO& channel_data, const String& membership_data) const {
    //TODO: channel id should be optional and if it's not provided, we should create hashed channel id
    String final_channel_id = channel_id;

    auto new_entity = channel_data.to_entity();
    new_entity.type = "group";
    auto created_channel = this->create_channel(final_channel_id, std::move(new_entity));

    String user_id;
    {
        auto pubnub_handle = this->pubnub->lock();
        user_id = pubnub_handle->get_user_id();

        //TODO: Add filter when it will be supported in C-Core
        String include_string = "custom,channel,totalCount,customChannel";
        String memberships_response = pubnub_handle->set_memberships(user_id, create_set_memberships_object(final_channel_id), include_string);
    }

    auto chat_service_shared = chat_service.lock();

    //TODO: Maybe current user should just be created in chat constructor and stored there all the time?
    User current_user = chat_service_shared->user_service->get_user(user_id);

    Membership host_membership = chat_service_shared->membership_service->create_membership_object(current_user, created_channel);
    std::vector<Membership> invitee_memberships = chat_service_shared->membership_service->invite_multiple_to_channel(final_channel_id, *created_channel.data, users);

    return std::make_tuple(created_channel, host_membership, invitee_memberships);
}

Channel ChannelService::create_channel(const String& channel_id, const ChannelEntity&& channel_entity) const {
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to create channel, channel_id is empty");
    }

    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->set_channel_metadata(channel_id, channel_entity.get_channel_metadata_json_string(channel_id));
    }

    return this->create_channel_object(std::make_pair(channel_id, channel_entity));
}

Channel ChannelService::get_channel(const String& channel_id) const {
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to get channel, channel_id is empty");
    }

    //We don't try to get this channel from entity repository here, as channel data could be updated on the server

    auto channel_response = [this, channel_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_channel_metadata(channel_id);
    }();

    auto parsed_response = Json::parse(channel_response);

    if(parsed_response.is_null()) {
        throw std::runtime_error("can't get channel, response is incorrect");
    }

    if(parsed_response["data"].is_null()) {
        throw std::runtime_error("can't get channel, response doesn't have data field");
    }

    return this->create_channel_object({channel_id, ChannelEntity::from_channel_response(parsed_response["data"])});
}

std::tuple<std::vector<Pubnub::Channel>, Pubnub::Page, int> ChannelService::get_channels(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    Pubnub::String include = "custom,totalCount";
    auto channels_response = [this, include, limit, filter, sort, page] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_all_channels_metadata(include, limit, filter, sort, page.next, page.prev);
    }();

    Json response_json = Json::parse(channels_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channels, response is incorrect");
    }

    Json channel_data_array_json = response_json["data"];
    std::vector<Channel> Channels;
   
    for (auto element : channel_data_array_json)
    {
         ChannelEntity new_channel_entity = ChannelEntity::from_json(element.dump());
         Channel channel = this->create_channel_object({String(element["id"]), std::move(new_channel_entity)});
 
         Channels.push_back(channel);
    }
    int total_count = response_json.get_int("totalCount").value_or(0);
    Page page_response({response_json.get_string("next").value_or(String("")), response_json.get_string("prev").value_or(String(""))});
    std::tuple<std::vector<Pubnub::Channel>, Pubnub::Page, int> return_tuple = std::make_tuple(Channels, page_response, total_count);

    return return_tuple;
}

Channel ChannelService::update_channel(const String& channel_id, ChannelDAO channel_data) const {
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to update channel, channel_id is empty");
    }

    auto entity = channel_data.to_entity();
    Channel channel = this->create_channel_object({channel_id, entity});

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->set_channel_metadata(channel_id, entity.get_channel_metadata_json_string(channel_id));

    return channel;
}

void ChannelService::delete_channel(const String& channel_id) const {
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to delete channel, channel_id is empty");
    }

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->remove_channel_metadata(channel_id);
}

std::vector<Message> ChannelService::get_channel_history(const String& channel_id, const String& start_timetoken, const String& end_timetoken, int count) const {
    auto fetch_history_response = [this, channel_id, start_timetoken, end_timetoken, count] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->fetch_history(channel_id, start_timetoken, end_timetoken, count);
    }();

    Json response_json = Json::parse(fetch_history_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get history, response is incorrect");
    }

    if(!response_json.contains("channels") && !response_json["channels"].contains(channel_id))
    {
        throw std::runtime_error("can't get history, response doesn't have channel info");
    }

    std::vector<Message> messages;

    auto entities = MessageEntity::from_history_json(response_json, channel_id);

    auto chat_service_shared = chat_service.lock();

    std::transform(entities.begin(), entities.end(), std::back_inserter(messages), [this, chat_service_shared](auto message) {
        return chat_service_shared->message_service->create_message_object(message);
    });

    return messages;
}

Channel ChannelService::pin_message_to_channel(const Message& message, const String& channel_id, const ChannelDAO& channel_data) const {
    return this->update_channel(
            channel_id,
            channel_data
                .to_entity()
                .pin_message({
                    message.message_data().channel_id,
                    message.timetoken()
                })
            );
}

Channel ChannelService::unpin_message_from_channel(const String& channel_id, const ChannelDAO& channel_data) const {
    return this->update_channel(channel_id, channel_data.to_entity().unpin_message());
}

#ifndef PN_CHAT_C_ABI
void ChannelService::connect(const String& channel_id, std::function<void(Message)> message_callback) const {
#else
std::vector<pubnub_v2_message> ChannelService::connect(const String& channel_id) const {
#endif // PN_CHAT_C_ABI
    auto messages = [this, channel_id] {
        auto pubnub_handle = this->pubnub->lock();

        return pubnub_handle->subscribe_to_channel_and_get_messages(channel_id);
    }();

#ifndef PN_CHAT_C_ABI
    if (auto chat = this->chat_service.lock()) {
        // First broadcast messages because they're not related to the new callback
        chat->callback_service->broadcast_messages(messages);
        chat->callback_service->register_message_callback(channel_id, message_callback);
    } else {
        throw std::runtime_error("Chat service is not available to connect to channel");
    }
#else
    return messages;
#endif // PN_CHAT_C_ABI
}

#ifndef PN_CHAT_C_ABI
void ChannelService::disconnect(const String& channel_id) const {
#else
std::vector<pubnub_v2_message> ChannelService::disconnect(const String& channel_id) const {
#endif // PN_CHAT_C_ABI
      auto pubnub_handle = this->pubnub->lock();
      auto messages = pubnub_handle->unsubscribe_from_channel_and_get_messages(channel_id);

#ifndef PN_CHAT_C_ABI
    if (auto chat = this->chat_service.lock()) {
        chat->callback_service->broadcast_messages(messages);
        chat->callback_service->remove_message_callback(channel_id);
    } else {
        throw std::runtime_error("Chat service is not available to connect to channel");
    }
#else
    return messages;
#endif // PN_CHAT_C_ABI
}

#ifndef PN_CHAT_C_ABI
void ChannelService::join(const String& channel_id, std::function<void(Message)> message_callback, const String& additional_params) const {
#else
std::vector<pubnub_v2_message> ChannelService::join(const String& channel_id, const String& additional_params) const {
#endif // PN_CHAT_C_ABI
    String set_object_string = create_set_memberships_object(channel_id, additional_params);

    {
        auto pubnub_handle = this->pubnub->lock();
        String user_id = pubnub_handle->get_user_id();
        pubnub_handle->set_memberships(user_id, set_object_string);
    }

#ifndef PN_CHAT_C_ABI
    this->connect(channel_id, message_callback);
#else
    return this->connect(channel_id);
#endif // PN_CHAT_C_ABI
}
    
#ifndef PN_CHAT_C_ABI
void ChannelService::leave(const String& channel_id) const {
#else
std::vector<pubnub_v2_message> ChannelService::leave(const String& channel_id) const {
#endif // PN_CHAT_C_ABI
    String remove_object_string = String("[{\"channel\": {\"id\": \"") + channel_id + String("\"}}]");

    {
        auto pubnub_handle = this->pubnub->lock();
        String user_id = pubnub_handle->get_user_id();
        pubnub_handle->remove_memberships(user_id, remove_object_string);
    }

#ifndef PN_CHAT_C_ABI
	this->disconnect(channel_id);
#else
    return this->disconnect(channel_id);
#endif // PN_CHAT_C_ABI
}

void ChannelService::send_text(const String& channel_id, const String& message, pubnub_chat_message_type message_type, const String& meta_data) const {
    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->publish(channel_id, chat_message_to_publish_string(message, message_type), meta_data);
}

void ChannelService::send_text(const Pubnub::String& channel_id, const Pubnub::String &message, const SendTextParamsInternal& text_params) const
{
    if(!text_params.quoted_message.timetoken.empty() && text_params.quoted_message.channel_id != channel_id)
    {
        throw std::invalid_argument("You cannot quote messages from other channels");
    }
    
    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->publish(channel_id, chat_message_to_publish_string(message, pubnub_chat_message_type::PCMT_TEXT), this->send_text_meta_from_params(text_params), text_params.store_in_history, text_params.send_by_post);
    }

    //TODO::This actually should be published message timetoken, but at the moment we don't have any way to get this from C-Core
    String mention_timetoken = get_now_timetoken();

    if(text_params.mentioned_users.size() > 0)
    {
        for(auto it = text_params.mentioned_users.begin(); it != text_params.mentioned_users.end(); it++)
        {
            this->emit_user_mention(channel_id, it->second.id, mention_timetoken, message);
        }
    }
}


void ChannelService::start_typing(const String& channel_id, ChannelDAO& channel_data) const {
    if(channel_data.get_entity().type == String("public"))
    {
        throw std::runtime_error("Typing indicators are not supported in Public chats");
    }

    if(channel_data.is_typing_sent()) {
        return;
    }

    auto chat_service_shared = chat_service.lock();

    auto user_id = [this] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_user_id();
    }();

    channel_data.start_typing(TYPING_TIMEOUT - TYPING_TIMEOUT_DIFFERENCE);

    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_TYPING, channel_id, Typing::payload(user_id, true));
}

void ChannelService::stop_typing(const String& channel_id, ChannelDAO& channel_data) const {
    if(channel_data.get_entity().type == String("public"))
    {
        throw std::runtime_error("Typing indicators are not supported in Public chats");
    }

    if (!channel_data.is_typing_sent()) {
        return;
    }

    channel_data.stop_typing();

    auto user_id = [this] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_user_id();
    }();
    auto chat_service_shared = chat_service.lock();

    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_TYPING, channel_id, Typing::payload(user_id, false));
}

std::function<void()> ChannelService::get_typing(const String& channel_id, ChannelDAO& channel_data, std::function<void(const std::vector<String>&)> typing_callback) const {
    auto typing_timeout = TYPING_TIMEOUT;
    std::function<void(Event)> internal_typing_callback = [&channel_data, typing_callback, typing_timeout] (Event event)
    {
        auto maybe_typing = Typing::typing_user_from_event(event);
        if(!maybe_typing.has_value()) {
            throw std::runtime_error("Can't get typing from payload");
        }

        auto user_id = maybe_typing.value().first;
        auto typing_value = maybe_typing.value().second;
       
        //stop typing
        if(!typing_value && channel_data.contains_typing_indicator(user_id)) {
            channel_data.stop_typing_indicator(user_id);
        }

        //start typing
        if(typing_value) {
            //Stop the old timer
            if(channel_data.contains_typing_indicator(user_id)) {
                channel_data.stop_typing_indicator(user_id);
            }
            
            //Create and start new timer
            channel_data.start_typing_indicator(user_id, typing_timeout, typing_callback);
        }
        typing_callback(channel_data.get_typing_indicators());
    };
    auto chat_service_shared = chat_service.lock();

#ifndef PN_CHAT_C_ABI
    return chat_service_shared->listen_for_events(channel_id, pubnub_chat_event_type::PCET_TYPING, internal_typing_callback);
#else
    auto messages = chat_service_shared->listen_for_events(channel_id, pubnub_chat_event_type::PCET_TYPING);
    // TODO: messages are not used in C ABI

    std::function<void()> dummy_return = [](){};
    return dummy_return;
#endif
}

Message ChannelService::get_pinned_message(const String& channel_id, const ChannelDAO& channel_data) const {
    Json custom_data_json = Json::parse(channel_data.get_entity().custom_data_json);
    if(!custom_data_json.contains("pinnedMessageTimetoken") || custom_data_json["pinnedMessageTimetoken"].is_null())
    {
        //TODO: I don't think we need to throw any error here, but we don't have empty message object.
        throw std::invalid_argument("there is no any pinned message");
    }

    String message_timetoken = custom_data_json.get_string("pinnedMessageTimetoken").value_or(String(""));
    String message_channel_id = custom_data_json.get_string("pinnedMessageChannelID").value_or(String(""));

    auto chat_service_shared = chat_service.lock();

    Message pinned_message = chat_service_shared->message_service->get_message(message_timetoken, message_channel_id);

    //TODO: also check here for pinned message in thread channela after implementing threads
    return pinned_message;
}

void ChannelService::emit_user_mention(const Pubnub::String &channel_id, const Pubnub::String &user_id, const Pubnub::String &timetoken, const Pubnub::String &text, const Pubnub::String &parent_channel_id) const
{
    auto chat_service_shared = chat_service.lock();
    json payload_json = json::object();
    payload_json["text"] = text.c_str();
    payload_json["messageTimetoken"] = timetoken.c_str();
    payload_json["channel"] = channel_id.c_str();
    if(!parent_channel_id.empty())
    {
        payload_json["parentChannel"] = parent_channel_id.c_str();
    }
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_MENTION, user_id, payload_json.dump());
}

std::vector<Pubnub::Channel> ChannelService::get_channel_suggestions(Pubnub::String text, int limit) const
{
    auto chat_shared = this->chat_service.lock();

    if(!chat_shared)
    {
        throw std::runtime_error("can't get users suggestions, chat service is invalid");
    }

    String cache_key = chat_shared->message_service->get_phrase_to_look_for(text);

    if(cache_key.empty())
    {
        return {};
    }

    //TODO:: cashe rezults here like in js

    String filter = "name LIKE \"" + cache_key + "*\"";

    auto get_channels_tuple = this->get_channels(filter, "", limit);
    return std::get<0>(get_channels_tuple);
}

std::vector<Pubnub::Membership> ChannelService::get_user_suggestions_for_channel(const String& channel_id, ChannelDAO& channel_data, Pubnub::String text, int limit) const
{
    auto chat_shared = this->chat_service.lock();

    if(!chat_shared)
    {
        throw std::runtime_error("can't get users suggestions, chat service is invalid");
    }

    String cache_key = chat_shared->message_service->get_phrase_to_look_for(text);

    if(cache_key.empty())
    {
        return {};
    }

    //TODO:: cashe rezults here like in js

    String filter = "uuid.name LIKE \"" + cache_key + "*\"";

    auto members_tuple = chat_shared->membership_service->get_channel_members(channel_id, channel_data, filter, "", limit);
    return std::get<0>(members_tuple);
}

std::function<void()> ChannelService::stream_updates(Pubnub::Channel calling_channel, std::function<void(Channel)> channel_callback) const
{
    auto pubnub_handle = this->pubnub->lock();

    auto chat = this->chat_service.lock();
    std::vector<String> channels_ids;
    std::function<void(Channel)> final_channel_callback = [=](Channel channel){
        auto updated_channel = this->update_channel_with_base(channel, calling_channel);
       
        channel_callback(updated_channel);
    };
    
    auto messages = pubnub_handle->subscribe_to_multiple_channels_and_get_messages({calling_channel.channel_id()});
    chat->callback_service->broadcast_messages(messages);

    chat->callback_service->register_channel_callback(calling_channel.channel_id(), final_channel_callback);

    //stop streaming callback
    std::function<void()> stop_streaming = [=](){
        chat->callback_service->remove_channel_callback(calling_channel.channel_id());
    };

    return stop_streaming;
}

std::function<void()> ChannelService::stream_updates_on(Pubnub::Channel calling_channel, const std::vector<Pubnub::Channel>& channels, std::function<void(std::vector<Channel>)> channel_callback) const
{
    if(channels.empty())
    {
        throw std::invalid_argument("Cannot stream channel updates on an empty list");
    }
    
    auto pubnub_handle = this->pubnub->lock();

    auto chat = this->chat_service.lock();
    std::vector<String> channels_ids;

   // calling_channel_entity.stream_updates_channels = channels;

    std::function<void(Channel)> single_channel_callback = [=](Channel channel){
        
        std::vector<Pubnub::Channel> updated_channels; 

        for(int i = 0; i < channels.size(); i++)
        {
            //Find channel that was updated and replace it in Entity stream channels
            auto stream_channel = channels[i];

            if(stream_channel.channel_id() == channel.channel_id())
            {
                ChannelEntity stream_channel_entity = ChannelDAO(stream_channel.channel_data()).to_entity();
                ChannelEntity channel_entity = ChannelDAO(channel.channel_data()).to_entity();
                std::pair<String, ChannelEntity> pair = std::make_pair(channel.channel_id(), ChannelEntity::from_base_and_updated_channel(stream_channel_entity, channel_entity));
                auto updated_channel = create_channel_object(pair);
                updated_channels.push_back(updated_channel);
            }
            else
            {
                updated_channels.push_back(channels[i]);
            }
        }
        //calling_channel_entity.stream_updates_channels = updated_channels;
        channel_callback(updated_channels);

    };
    
    for(auto channel : channels)
    {
        channels_ids.push_back(channel.channel_id());
        chat->callback_service->register_channel_callback(channel.channel_id(), single_channel_callback);
    }
    

    auto messages = pubnub_handle->subscribe_to_multiple_channels_and_get_messages(channels_ids);
    chat->callback_service->broadcast_messages(messages);

    //stop streaming callback
    std::function<void()> stop_streaming = [=, &channels_ids](){
        for(auto id : channels_ids)
        {
            chat->callback_service->remove_channel_callback(id);
        }
    };

    return stop_streaming;
}


std::function<void()> ChannelService::stream_read_receipts(const Pubnub::String& channel_id, const ChannelDAO& channel_data, std::function<void(std::map<Pubnub::String, std::vector<Pubnub::String>, Pubnub::StringComparer>)> read_receipts_callback) const
{
    if(channel_data.get_entity().type == String("public"))
    {
        throw std::runtime_error("Read receipts are not supported in Public chats");
    }

    auto chat_service_shared = chat_service.lock();

    auto generate_receipts = [=](std::map<String, String, StringComparer> in_timetoken_per_user) -> std::map<Pubnub::String, std::vector<Pubnub::String>, Pubnub::StringComparer>
    {
        std::map<Pubnub::String, std::vector<Pubnub::String>, Pubnub::StringComparer> receipts;

        for(auto it = in_timetoken_per_user.begin(); it != in_timetoken_per_user.end(); it++)
        {
            //If there is no key with such timetoken yet, just add empty array
            if(receipts.find(it->second) == receipts.end())
            {
                receipts[it->second] = {};
            }
            receipts[it->second].push_back(it->first);
        }

        return receipts;

    };
    std::map<String, String, StringComparer> timetoken_per_user;

    auto members_tuple = chat_service_shared->membership_service->get_channel_members(channel_id, channel_data);
    auto channel_members = std::get<0>(members_tuple);
    for(auto membership : channel_members)
    {
        String last_read_timetoken = membership.last_read_message_timetoken();
        if(last_read_timetoken.empty())
        {
            continue;
        }

        timetoken_per_user[membership.user.user_id()] =  last_read_timetoken;
        
    }

    read_receipts_callback(generate_receipts(timetoken_per_user));

    auto receipt_event_callback = [=, &timetoken_per_user](const Pubnub::Event& event){

        json payload_json = json::parse(event.payload);

        timetoken_per_user[String(payload_json["user_id"])] = String(payload_json["messageTimetoken"]);

        read_receipts_callback(generate_receipts(timetoken_per_user));
    };

#ifndef PN_CHAT_C_ABI
    return chat_service_shared->listen_for_events(channel_id, pubnub_chat_event_type::PCET_RECEPIT, receipt_event_callback);
#else
chat_service_shared->listen_for_events(channel_id, pubnub_chat_event_type::PCET_RECEPIT);
    std::function<void()> dumy_return = [](){};
    return dumy_return;
#endif
}

String ChannelService::get_thread_id(const Pubnub::Message& message) const
{
    return MESSAGE_THREAD_ID_PREFIX + "_" + message.message_data().channel_id + "_" + message.timetoken();
}

ThreadChannel ChannelService::create_thread_channel(const Pubnub::Message& message) const
{
    String message_channel_id = message.message_data().channel_id;
    if(string_starts_with(message_channel_id, MESSAGE_THREAD_ID_PREFIX))
    {
        throw std::invalid_argument("Only one level of thread nesting is allowed");
    }

    if(message.deleted())
    {
        throw std::invalid_argument("You cannot create threads on deleted messages");
    }

    String thread_id = this->get_thread_id(message);
    bool is_existing_thread = true;

    try 
    {
        this->get_channel(thread_id);
    }
    catch (...)
    {
        //If there is error in getting channel it means that there is no such channel
        is_existing_thread = false;
    }

    if(is_existing_thread)
    {
        throw std::runtime_error("Thread for this message already exists");
    }

    String thread_description = String("Thread on channel ") + message.message_data().channel_id + String(" with message timetoken ") + message.timetoken();

    ChatChannelData thread_channel_data;
    thread_channel_data.description = thread_description;
    ChannelDAO channel_dao(thread_channel_data);

    auto channel_entity = channel_dao.to_entity();

    auto new_thread_channel = create_thread_channel_object({thread_id, channel_entity}, message);
    //This thread is not created yet, it will be after sending the first message to this thread.
    //This is to avoid creating threads without any messages
    new_thread_channel.set_is_thread_created(false);

    return new_thread_channel;

}

ThreadChannel ChannelService::get_thread_channel(const Pubnub::Message& message) const
{
    String thread_id = this->get_thread_id(message);

    auto channel_response = [this, thread_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_channel_metadata(thread_id);
    }();

    auto parsed_response = Json::parse(channel_response);

    if(parsed_response.is_null()) {
        throw std::runtime_error("can't get thread channel, response is incorrect");
    }

    if(parsed_response["data"].is_null()) {
        throw std::runtime_error("can't get thread channel, response doesn't have data field");
    }

    return this->create_thread_channel_object({thread_id, ChannelEntity::from_channel_response(parsed_response["data"])}, message);

}

void ChannelService::confirm_creating_thread(const Pubnub::ThreadChannel& thread_channel) const
{
    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->set_channel_metadata(thread_channel.channel_id(), thread_channel.data->get_entity().get_channel_metadata_json_string(thread_channel.channel_id()));
    String message_action_value = String("\"") + thread_channel.channel_id() + String("\"");
    pubnub_handle->add_message_action(thread_channel.parent_message().message_data().channel_id, thread_channel.parent_message().timetoken(), message_action_type_to_string(pubnub_message_action_type::PMAT_ThreadRootId), message_action_value);
}

bool ChannelService::has_thread_channel(const Pubnub::Message &message) const
{
    bool found_thread_root = false;
    for(auto &message_action : message.message_data().message_actions)
    {
        if(message_action.type == pubnub_message_action_type::PMAT_ThreadRootId && !message_action.value.empty())
        {
            found_thread_root = true;
            break;
        }
    }

    return found_thread_root;
}

void ChannelService::remove_thread_channel(const Pubnub::Message &message) const
{
    if(!message.has_thread())
    {
        throw std::invalid_argument("There is no thread to be deleted");
    }

    MessageAction thread_message_action;
    for(auto &message_action : message.message_data().message_actions)
    {
        if(message_action.type == pubnub_message_action_type::PMAT_ThreadRootId)
        {
            thread_message_action = message_action;
        }
    }

    String thread_id = this->get_thread_id(message);

    Channel thread_channel = this->get_channel(thread_id);

    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->remove_message_action(message.message_data().channel_id, message.timetoken(), thread_message_action.timetoken);
    }

    thread_channel.delete_channel();
}

Pubnub::ThreadChannel ChannelService::pin_message_to_thread_channel(const Pubnub::ThreadMessage &message, const Pubnub::ThreadChannel& thread_channel) const
{
    auto new_channel = this->pin_message_to_channel(message, thread_channel.channel_id(), *thread_channel.data);
    return create_thread_channel_object({thread_channel.channel_id(), new_channel.data->get_entity()}, thread_channel.parent_message());
}

Pubnub::ThreadChannel ChannelService::unpin_message_from_thread_channel(const Pubnub::ThreadChannel& thread_channel) const
{
    auto new_channel = this->unpin_message_from_channel(thread_channel.channel_id(), *thread_channel.data);
    return create_thread_channel_object({thread_channel.channel_id(), new_channel.data->get_entity()}, thread_channel.parent_message());
}

std::vector<Pubnub::ThreadMessage> ChannelService::get_thread_channel_history(const Pubnub::String &channel_id, const Pubnub::String &start_timetoken, const Pubnub::String &end_timetoken, int count, const Pubnub::String &parent_channel_id) const
{
    auto base_messages = this->get_channel_history(channel_id, start_timetoken, end_timetoken, count);
    std::vector<Pubnub::ThreadMessage> thread_messages;

    auto chat_service_shared = chat_service.lock();
    for(auto base_message : base_messages)
    {
        thread_messages.push_back(chat_service_shared->message_service->create_thread_message_object(base_message, parent_channel_id));
    }
    return thread_messages;
}

Channel ChannelService::create_channel_object(std::pair<String, ChannelEntity> channel_data) const
{
    if (auto chat = this->chat_service.lock()) {
        return Channel(
                channel_data.first,
                chat,
                shared_from_this(),
                chat->presence_service,
                chat->restrictions_service,
                chat->message_service,
                chat->membership_service,
                std::make_unique<ChannelDAO>(channel_data.second)
            );
    } else {
        throw std::runtime_error("Chat service is not available to create channel object");
    }
}

ThreadChannel ChannelService::create_thread_channel_object(std::pair<String, ChannelEntity> channel_data, Pubnub::Message parent_message) const
{
    if (auto chat = this->chat_service.lock()) {
        return ThreadChannel(
                channel_data.first,
                chat,
                shared_from_this(),
                chat->presence_service,
                chat->restrictions_service,
                chat->message_service,
                chat->membership_service,
                std::make_unique<ChannelDAO>(channel_data.second),
                parent_message.message_data().channel_id,
                parent_message
            );
    } else {
        throw std::runtime_error("Chat service is not available to create thread channel object");
    }
}

String ChannelService::send_text_meta_from_params(const SendTextParamsInternal& text_params) const
{
    bool any_data_added = !text_params.meta.empty();
    String meta = text_params.meta.empty() ? "{}" : text_params.meta;
    json message_json = json::parse(meta);

    //mentioned users
    if(text_params.mentioned_users.size() > 0)
    {
        json mentioned_users_json;
        auto mentioned_users = text_params.mentioned_users;

        for(auto it = mentioned_users.begin(); it != mentioned_users.end(); it++)
        {
            json mentioned_user_json;
            mentioned_user_json["id"] = String(it->second.id).c_str();
            mentioned_user_json["name"] = String(it->second.name).c_str();
            String key = std::to_string(it->first);
            mentioned_users_json[key] = mentioned_user_json;
        }

        message_json["mentionedUsers"] = mentioned_users_json;
        any_data_added = true;
    }

    //referenced channels
    if(text_params.referenced_channels.size() > 0)
    {
        json referenced_channels_json;
        auto referenced_channels = text_params.referenced_channels;

        for(auto it = referenced_channels.begin(); it != referenced_channels.end(); it++)
        {
            json referenced_channel_json;
            referenced_channel_json["id"] = String(it->second.id).c_str();
            referenced_channel_json["name"] = String(it->second.name).c_str();
            String key = std::to_string(it->first);
            referenced_channels_json[key] = referenced_channel_json;
        }

        message_json["referencedChannels"] = referenced_channels_json;
        any_data_added = true;
    }

    //text links
    if(text_params.text_links.size() > 0)
    {
        json text_links_json = json::array();
        auto text_links = text_params.text_links;

        for(auto text_link : text_links)
        {
            json text_link_json;
            text_link_json["start_index"] = text_link.start_index;
            text_link_json["end_index"] = text_link.end_index;
            text_link_json["link"] = text_link.link.c_str();

            text_links_json.push_back(text_link_json);
        }

        message_json["textLinks"] = text_links_json;
        any_data_added = true;
    }

    //quoted message
    if(!text_params.quoted_message.timetoken.empty())
    {
        json quoted_message_json;
        quoted_message_json["timetoken"] = text_params.quoted_message.timetoken.c_str();
        quoted_message_json["text"] = text_params.quoted_message.text.c_str();
        quoted_message_json["userId"] = text_params.quoted_message.user_id.c_str();
        quoted_message_json["channelId"] = text_params.quoted_message.channel_id.c_str();
        message_json["quotedMessage"] = quoted_message_json;

        any_data_added = true;
    }

    String final_metadata = any_data_added ? String(message_json.dump()) : String("");

	return final_metadata;
}

Pubnub::Channel ChannelService::update_channel_with_base(const Pubnub::Channel& channel, const Pubnub::Channel& base_channel) const
{
    ChannelEntity base_entity = ChannelDAO(base_channel.channel_data()).to_entity();
    ChannelEntity channel_entity = ChannelDAO(channel.channel_data()).to_entity();

    return create_channel_object(
            {channel.channel_id(), ChannelEntity::from_base_and_updated_channel(base_entity, channel_entity)});
}

#ifdef PN_CHAT_C_ABI
void ChannelService::stream_updates_on(const std::vector<Pubnub::String>& channel_ids) const
{
    //TODO:: C_ABI Way
    if(channel_ids.empty())
    {
        throw std::invalid_argument("Cannot stream channel updates on an empty list");
    }
    
    auto pubnub_handle = this->pubnub->lock();
    auto messages = pubnub_handle->subscribe_to_multiple_channels_and_get_messages(channel_ids);
}

void ChannelService::stream_read_receipts(const Pubnub::String& channel_id, const ChannelDAO& channel_data) const
{
    //TODO:: C_ABI Way
    // if(channel_data.get_entity().type == String("public"))
    // {
    //     throw std::runtime_error("Read receipts are not supported in Public chats");
    // }

    // auto chat_service_shared = chat_service.lock();

    // auto generate_receipts = [=](std::map<String, String, StringComparer> in_timetoken_per_user) -> std::map<Pubnub::String, std::vector<Pubnub::String>, Pubnub::StringComparer>
    // {
    //     std::map<Pubnub::String, std::vector<Pubnub::String>, Pubnub::StringComparer> receipts;

    //     for(auto it = in_timetoken_per_user.begin(); it != in_timetoken_per_user.end(); it++)
    //     {
    //         //If there is no key with such timetoken yet, just add empty array
    //         if(receipts.find(it->second) == receipts.end())
    //         {
    //             receipts[it->second] = {};
    //         }
    //         receipts[it->second].push_back(it->first);
    //     }

    //     return receipts;

    // };
    // std::map<String, String, StringComparer> timetoken_per_user;

    // auto channel_members = chat_service_shared->membership_service->get_channel_members(channel_id, channel_data);

    // for(auto membership : channel_members)
    // {
    //     String last_read_timetoken = membership.last_read_message_timetoken();
    //     if(last_read_timetoken.empty())
    //     {
    //         continue;
    //     }

    //     timetoken_per_user[membership.user.user_id()] =  last_read_timetoken;
        
    // }

    // read_receipts_callback(generate_receipts(timetoken_per_user));

    // auto receipt_event_callback = [=, &timetoken_per_user](const Pubnub::Event& event){

    //     json payload_json = json::parse(event.payload);

    //     timetoken_per_user[String(payload_json["user_id"])] = String(payload_json["messageTimetoken"]);

    //     read_receipts_callback(generate_receipts(timetoken_per_user));
    // };

    // auto messages = chat_service_shared->listen_for_events(channel_id, pubnub_chat_event_type::PCET_RECEPIT);
    // TODO: messages are not used in C ABI 
}

#endif // PN_CHAT_C_ABI
