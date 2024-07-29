#include "channel_service.hpp"
#include "application/dao/channel_dao.hpp"
#include "chat_service.hpp"
#include "domain/channel_entity.hpp"
#include "domain/json.hpp"
#include "domain/typing.hpp"
#include "user_service.hpp"
#include "membership_service.hpp"
#include "message_service.hpp"
#include "message.hpp"
#include "pubnub_chat/thread_channel.hpp"
#include "const_values.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "infra/timer.hpp"
#include "chat_helpers.hpp"
#include "nlohmann/json.hpp"
#include "application/callback_service.hpp"
#ifdef PN_CHAT_C_ABI
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

std::vector<Channel> ChannelService::get_channels(const String& include, int limit, const String& start, const String& end) const {
    auto channels_response = [this, include, limit, start, end] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_all_channels_metadata(include, limit, start, end);
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

    return Channels;
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

Channel ChannelService::pin_message_to_channel(const Message& message, const String& channel_id, const ChannelDAO& channel_data) const {
    return this->update_channel(
            channel_id,
            channel_data
                .to_entity()
                .pin_message({
                    channel_id,
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
std::vector<Pubnub::String> ChannelService::connect(const String& channel_id) const {
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
    auto messages_strings = std::vector<String>();
    std::transform(messages.begin(), messages.end(), std::back_inserter(messages_strings), [](pubnub_v2_message message) {
        return Parsers::PubnubJson::to_string(message);
    });
    return messages_strings;
#endif // PN_CHAT_C_ABI
}

void ChannelService::disconnect(const String& channel_id) const {
      auto pubnub_handle = this->pubnub->lock();
      auto messages = pubnub_handle->unsubscribe_from_channel_and_get_messages(channel_id);

     // TODO: C ABI way
#ifndef PN_CHAT_C_ABI
    if (auto chat = this->chat_service.lock()) {
        chat->callback_service->broadcast_messages(messages);
        chat->callback_service->remove_message_callback(channel_id);
    } else {
        throw std::runtime_error("Chat service is not available to connect to channel");
    }
#endif // PN_CHAT_C_ABI
}

#ifndef PN_CHAT_C_ABI
void ChannelService::join(const String& channel_id, std::function<void(Message)> message_callback, const String& additional_params) const {
#else
std::vector<Pubnub::String> ChannelService::join(const String& channel_id, const String& additional_params) const {
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

void ChannelService::leave(const String& channel_id) const {
    String remove_object_string = String("[{\"channel\": {\"id\": \"") + channel_id + String("\"}}]");

    {
        auto pubnub_handle = this->pubnub->lock();
        String user_id = pubnub_handle->get_user_id();
        pubnub_handle->remove_memberships(user_id, remove_object_string);
    }

	this->disconnect(channel_id);
}

void ChannelService::send_text(const String& channel_id, const String& message, pubnub_chat_message_type message_type, const String& meta_data) const {
    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->publish(channel_id, chat_message_to_publish_string(message, message_type), meta_data);
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

void ChannelService::get_typing(const String& channel_id, ChannelDAO& channel_data, std::function<void(const std::vector<String>&)> typing_callback) const {
    auto typing_timeout = TYPING_TIMEOUT;
    std::function<void(String)> internal_typing_callback = [&channel_data, typing_callback, typing_timeout] (String event_string)
    {
        auto maybe_typing = Typing::typing_user_from_payload(event_string);
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

    chat_service_shared->listen_for_events(channel_id, pubnub_chat_event_type::PCET_TYPING, internal_typing_callback);
}

Message ChannelService::get_pinned_message(const String& channel_id, const ChannelDAO& channel_data) const {
    json custom_data_json = json::parse(channel_data.get_entity().custom_data_json);
    if(!custom_data_json.contains("pinnedMessageTimetoken") || custom_data_json["pinnedMessageTimetoken"].is_null())
    {
        //TODO: I don't think we need to throw any error here, but we don't have empty message object.
        throw std::invalid_argument("there is no any pinned message");
    }

    String message_timetoken = custom_data_json["pinnedMessageTimetoken"].dump();
    message_timetoken.erase(0, 1);
    message_timetoken.erase(message_timetoken.length() - 1, 1);

    auto chat_service_shared = chat_service.lock();

    Message pinned_message = chat_service_shared->message_service->get_message(message_timetoken, channel_id);

    //TODO: also check here for pinned message in thread channela after implementing threads
    return pinned_message;
}

void ChannelService::stream_updates_on(const std::vector<Pubnub::String>& channel_ids, std::function<void(Channel)> channel_callback) const
{
    if(channel_ids.empty())
    {
        throw std::invalid_argument("Cannot stream channel updates on an empty list");
    }
    
    auto pubnub_handle = this->pubnub->lock();

#ifndef PN_CHAT_C_ABI
    // chat is not needed in C ABI to stream updates
    if (auto chat = this->chat_service.lock()) {
#endif // PN_CHAT_C_ABI
        std::vector<String> channels_ids;

        for(auto channel : channel_ids)
        {
#ifndef PN_CHAT_C_ABI
            chat->callback_service->register_channel_callback(channel, channel_callback);
#endif // PN_CHAT_C_ABI
        }
        
        auto messages = pubnub_handle->subscribe_to_multiple_channels_and_get_messages(channels_ids);

        // TODO: C ABI way
#ifndef PN_CHAT_C_ABI
        chat->callback_service->broadcast_messages(messages);
    }
#endif // PN_CHAT_C_ABI
}

void ChannelService::stream_read_receipts(const Pubnub::String& channel_id, const ChannelDAO& channel_data, std::function<void(std::map<Pubnub::String, std::vector<Pubnub::String>, Pubnub::StringComparer>)> read_receipts_callback) const
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

    auto channel_members = chat_service_shared->membership_service->get_channel_members(channel_id, channel_data);

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

    auto receipt_event_callback = [=, &timetoken_per_user](const Pubnub::String& event){

        json payload_json = json::parse(event);

        timetoken_per_user[String(payload_json["user_id"])] = String(payload_json["messageTimetoken"]);

        read_receipts_callback(generate_receipts(timetoken_per_user));
    };

    chat_service_shared->listen_for_events(channel_id, pubnub_chat_event_type::PCET_RECEPIT, receipt_event_callback);
}

String ChannelService::get_thread_id(const Pubnub::Message& message)
{
    return MESSAGE_THREAD_ID_PREFIX + "_" + message.message_data().channel_id + "_" + message.timetoken();
}

ThreadChannel ChannelService::create_thread_channel(const Pubnub::Message& message)
{
    if(string_starts_with(message.message_data().channel_id, MESSAGE_THREAD_ID_PREFIX))
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

    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->set_channel_metadata(thread_id, channel_entity.get_channel_metadata_json_string(thread_id));
        String message_action_value = String("{\"value\": \"}") + thread_id + String("\"}");
        pubnub_handle->add_message_action(message.message_data().channel_id, message.timetoken(), "threadRootId", message_action_value);
    }
    //TODO:: this code is very complex here in JS CHAT. Check with Piotr if it does everything that it should do.

    return new_thread_channel;

}

ThreadChannel ChannelService::get_thread_channel(const Pubnub::Message& message)
{
    String thread_id = this->get_thread_id(message);

    throw std::runtime_error("I will continue work here");
    
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
