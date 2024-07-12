#include "channel_service.hpp"
#include "chat_service.hpp"
#include "domain/json.hpp"
#include "user_service.hpp"
#include "membership_service.hpp"
#include "message_service.hpp"
#include "presentation/message.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "infra/timer.hpp"
#include "chat_helpers.hpp"
#include "nlohmann/json.hpp"
#include "application/callback_service.hpp"

using namespace Pubnub;
using json = nlohmann::json;

ChannelService::ChannelService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
    entity_repository(entity_repository),
    chat_service(chat_service)
{}

ChatChannelData ChannelService::get_channel_data(String channel_id)
{
    auto maybe_channel = this->entity_repository->get_channel_entities().get(channel_id);

    if (!maybe_channel.has_value()) 
    {
        throw std::invalid_argument("Failed to get channel data, there is no channel with this id");
    }

    return presentation_data_from_domain(maybe_channel.value());
}

Channel ChannelService::create_public_conversation(String channel_id, ChatChannelData data)
{
    data.type = "public";
    return create_channel(channel_id, data);
}

std::tuple<Channel, Membership, std::vector<Membership>> ChannelService::create_direct_conversation(User user, String channel_id, ChatChannelData channel_data, String membership_data)
{
    //TODO: channel id should be optional and if it's not provided, we should create hashed channel id
    String final_channel_id = channel_id;

    channel_data.type = "direct";
    auto created_channel = this->create_channel(final_channel_id, channel_data);
    Pubnub::String user_id;

    {
        auto pubnub_handle = this->pubnub->lock();

        //TODO: Add filter when it will be supported in C-Core
        String include_string = "totalCount,customFields,channelFields,customChannelFields";
        user_id = pubnub_handle->get_user_id();
        pubnub_handle->set_memberships(pubnub_handle->get_user_id(), create_set_memberships_object(final_channel_id), include_string);
    }

    auto chat_service_shared = chat_service.lock();

    //TODO: Maybe current user should just be created in chat constructor and stored there all the time?
    User current_user = chat_service_shared->user_service->get_user(user_id);

    Membership host_membership = chat_service_shared->membership_service->create_presentation_object(current_user, created_channel);
    Membership invitee_membership = chat_service_shared->membership_service->invite_to_channel(final_channel_id, user);

    std::tuple<Channel, Membership, std::vector<Membership>> final_tuple(created_channel, host_membership, {invitee_membership});

    return final_tuple;
}

std::tuple<Channel, Membership, std::vector<Membership>> ChannelService::create_group_conversation(std::vector<User> users, String channel_id, ChatChannelData channel_data, String membership_data)
{
    //TODO: channel id should be optional and if it's not provided, we should create hashed channel id
    String final_channel_id = channel_id;

    channel_data.type = "group";
    auto created_channel = this->create_channel(final_channel_id, channel_data);

    Pubnub::String user_id;
    {
        auto pubnub_handle = this->pubnub->lock();
        user_id = pubnub_handle->get_user_id();

        //TODO: Add filter when it will be supported in C-Core
        String include_string = "totalCount,customFields,channelFields,customChannelFields";
        String memberships_response = pubnub_handle->set_memberships(user_id, create_set_memberships_object(final_channel_id), include_string);
    }

    auto chat_service_shared = chat_service.lock();

    //TODO: Maybe current user should just be created in chat constructor and stored there all the time?
    User current_user = chat_service_shared->user_service->get_user(user_id);

    Membership host_membership = chat_service_shared->membership_service->create_presentation_object(current_user, created_channel);
    std::vector<Membership> invitee_memberships = chat_service_shared->membership_service->invite_multiple_to_channel(final_channel_id, users);

    std::tuple<Channel, Membership, std::vector<Membership>> final_tuple(created_channel, host_membership, invitee_memberships);

    return final_tuple;
}

Channel ChannelService::create_channel(String channel_id, ChatChannelData data) {

    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to create channel, channel_id is empty");
    }

    auto maybe_channel = this->entity_repository->get_channel_entities().get(channel_id);

    if (maybe_channel.has_value()) {
        return create_presentation_object(channel_id);
    }

    ChannelEntity new_channel_entity = create_domain_from_presentation_data(channel_id, data);

    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->set_channel_metadata(channel_id, new_channel_entity.get_channel_metadata_json_string(channel_id));
    }

    return this->create_channel_object(std::make_pair(channel_id, new_channel_entity));
}

Channel ChannelService::get_channel(String channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to get channel, channel_id is empty");
    }

    //We don't try to get this channel from entity repository here, as channel data could be updated on the server

    auto channel_response = [this, channel_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_channel_metadata(channel_id);
    }();

    ChannelEntity new_channel_entity = create_domain_from_channel_response(channel_response);
    Channel channel = create_presentation_object(channel_id);

    //Add or update channel_entity to repository
    entity_repository->get_channel_entities().update_or_insert(channel_id, new_channel_entity);

    return channel;
}

std::vector<Channel> ChannelService::get_channels(String include, int limit, String start, String end)
{
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
        Channel channel = create_presentation_object(String(element["id"]));

        entity_repository->get_channel_entities().update_or_insert(String(element["id"]), new_channel_entity);

        Channels.push_back(channel);
   }

    return Channels;
}

Channel ChannelService::update_channel(String channel_id, ChatChannelData channel_data)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to update channel, channel_id is empty");
    }

    Channel channel = create_presentation_object(channel_id);

    ChannelEntity new_channel_entity = create_domain_from_presentation_data(channel_id, channel_data);

    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->set_channel_metadata(channel_id, new_channel_entity.get_channel_metadata_json_string(channel_id));
    }

    //Add channel_entity to repository
    entity_repository->get_channel_entities().update_or_insert(channel_id, new_channel_entity);
    
    return channel;
}

void ChannelService::delete_channel(String channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to delete channel, channel_id is empty");
    }

    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->remove_channel_metadata(channel_id);
    }

    //Also remove this channel from entities repository
    entity_repository->get_channel_entities().remove(channel_id);
}

void ChannelService::pin_message_to_channel(Message message, Channel channel)
{
    String custom_channel_data;
    channel.channel_data().custom_data_json.empty() ?  custom_channel_data = "{}" :  custom_channel_data = channel.channel_data().custom_data_json;

    json custom_data_json = json::parse(custom_channel_data);
    custom_data_json["pinnedMessageTimetoken"] = message.timetoken().c_str();
    custom_data_json["pinnedMessageChannelID"] = channel.channel_id().c_str();

    ChatChannelData new_channel_data = channel.channel_data();
    new_channel_data.custom_data_json = custom_data_json.dump();

    this->update_channel(channel.channel_id(), new_channel_data);
}

void ChannelService::unpin_message_from_channel(Channel channel)
{
    String custom_channel_data = channel.channel_data().custom_data_json;
    if(!custom_channel_data.empty())
    {
        json custom_data_json = json::parse(channel.channel_data().custom_data_json);
        custom_data_json.erase("pinnedMessageTimetoken");
        custom_data_json.erase("pinnedMessageChannelID");
        custom_channel_data = custom_data_json.dump();
    }

    ChatChannelData new_channel_data = channel.channel_data();
    new_channel_data.custom_data_json = custom_channel_data;
    this->update_channel(channel.channel_id(), new_channel_data);
}

void ChannelService::connect(String channel_id, std::function<void(Message)> message_callback)
{
    auto messages = [this, channel_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->subscribe_to_channel_and_get_messages(channel_id);
    }();

    // TODO: C ABI way
#ifndef PN_CHAT_C_ABI
    if (auto chat = this->chat_service.lock()) {
        // First broadcast messages because they're not related to the new callback
        chat->callback_service->broadcast_messages(messages);
        chat->callback_service->register_message_callback(channel_id, message_callback);
    } else {
        throw std::runtime_error("Chat service is not available to connect to channel");
    }
#endif // PN_CHAT_C_ABI
}

void ChannelService::disconnect(String channel_id)
{
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

void ChannelService::join(String channel_id, std::function<void(Message)> message_callback, String additional_params)
{
    String include_string = "totalCount,customFields,channelFields,customChannelFields";
    String set_object_string = create_set_memberships_object(channel_id, additional_params);

    {
        auto pubnub_handle = this->pubnub->lock();
        String user_id = pubnub_handle->get_user_id();
        pubnub_handle->set_memberships(user_id, set_object_string);
    }

    this->connect(channel_id, message_callback);
}

void ChannelService::leave(String channel_id)
{
    String remove_object_string = String("[{\"channel\": {\"id\": \"") + channel_id + String("\"}}]");

    {
        auto pubnub_handle = this->pubnub->lock();
        String user_id = pubnub_handle->get_user_id();
        pubnub_handle->remove_memberships(user_id, remove_object_string);
    }

	this->disconnect(channel_id);
}

void ChannelService::send_text(String channel_id, String message, pubnub_chat_message_type message_type, String meta_data)
{
    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->publish(channel_id, chat_message_to_publish_string(message, message_type), meta_data);
}

void ChannelService::start_typing(String channel_id)
{
    ChannelEntity channel_entity = entity_repository->get_channel_entities().get(channel_id).value();

    if(presentation_data_from_domain(channel_entity).type == String("public"))
    {
        throw std::runtime_error("Typing indicators are not supported in Public chats");
    }
    if(channel_entity.typing_sent) return;

    auto chat_service_shared = chat_service.lock();

    auto user_id = [this] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_user_id();
    }();

    channel_entity.set_typing_sent(true);
    channel_entity.set_typing_sent_timer(Timer());
    channel_entity.typing_sent_timer.start(TYPING_TIMEOUT - 1000, [&channel_entity](){
         channel_entity.set_typing_sent(false);
    });
    
    channel_entity.set_typing_sent(true);
    String event_payload = String("{\"value\": true, \"userId\": \"") + user_id + String("\"}");
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_TYPING, channel_id, event_payload);
}

void ChannelService::stop_typing(String channel_id)
{
    ChannelEntity channel_entity = entity_repository->get_channel_entities().get(channel_id).value();

    if(presentation_data_from_domain(channel_entity).type == String("public"))
    {
        throw std::runtime_error("Typing indicators are not supported in Public chats");
    }
    channel_entity.typing_sent_timer.stop();

    if(!channel_entity.typing_sent) return;

    auto user_id = [this] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_user_id();
    }();

    auto chat_service_shared = chat_service.lock();

    channel_entity.set_typing_sent(false);
    String event_payload = String("{\"value\": false, \"userId\": \"") + user_id + String("\"}");
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_TYPING, channel_id, event_payload);
}

void ChannelService::get_typing(String channel_id, std::function<void(std::vector<String>)> typing_callback)
{
    std::function<void(String)> internal_typing_callback = [=](String event_string)
    {
        ChannelEntity channel_entity = entity_repository->get_channel_entities().get(channel_id).value();

        json event_json = json::parse(event_string);
        String user_id = event_json["userId"].dump();
        user_id.erase(0, 1);
        user_id.erase(user_id.length() - 1, 1);
        bool typing_value = event_json["value"];
        
        //stop typing
        if(!typing_value && channel_entity.typing_indicators.find(user_id) != channel_entity.typing_indicators.end())
        {
            channel_entity.typing_indicators[user_id].stop();
            channel_entity.typing_indicators.erase(user_id);
        }
        //start typing
        if(typing_value)
        {
            //Stop the old timer
            if(channel_entity.typing_indicators.find(user_id) != channel_entity.typing_indicators.end())
            {
                channel_entity.typing_indicators[user_id].stop();
            }
            
            //Create and start new timer
            Timer new_timer;
            new_timer.start(TYPING_TIMEOUT, [=, &channel_entity]()
            {
                channel_entity.typing_indicators.erase(user_id);
                typing_callback(getKeys(channel_entity.typing_indicators));
            });
            channel_entity.typing_indicators[user_id] = new_timer;
        }
        typing_callback(getKeys(channel_entity.typing_indicators));
    };
    auto chat_service_shared = chat_service.lock();

    chat_service_shared->listen_for_events(channel_id, pubnub_chat_event_type::PCET_TYPING, internal_typing_callback);
}

Pubnub::Message ChannelService::get_pinned_message(Pubnub::String channel_id)
{
    Channel channel = create_presentation_object(channel_id);
    json custom_data_json = json::parse(channel.channel_data().custom_data_json);
    if(!custom_data_json.contains("pinnedMessageTimetoken") || custom_data_json["pinnedMessageTimetoken"].is_null())
    {
        //TODO: I don't think we need to throw any error here, but we don't have empty message object.
        throw std::invalid_argument("there is no any pinned message");
    }

    String message_timetoken = custom_data_json["pinnedMessageTimetoken"].dump();
    message_timetoken.erase(0, 1);
    message_timetoken.erase(message_timetoken.length() - 1, 1);

    auto chat_service_shared = chat_service.lock();

    Pubnub::Message pinned_message = chat_service_shared->message_service->get_message(message_timetoken, channel_id);

    //TODO: also check here for pinned message in thread channela after implementing threads
    return pinned_message;
}

void ChannelService::stream_updates_on(std::vector<Pubnub::Channel> channels, std::function<void(Pubnub::Channel)> channel_callback)
{
    if(channels.empty())
    {
        throw std::invalid_argument("Cannot stream channel updates on an empty list");
    }
    
    auto pubnub_handle = this->pubnub->lock();

#ifndef PN_CHAT_C_ABI
    // chat is not needed in C ABI to stream updates
    if (auto chat = this->chat_service.lock()) {
#endif // PN_CHAT_C_ABI
        for(auto channel : channels)
        {
            auto messages = pubnub_handle->subscribe_to_channel_and_get_messages(channel.channel_id());

            // TODO: C ABI way
#ifndef PN_CHAT_C_ABI
            // First broadcast messages because they're not related to the new callback 
            chat->callback_service->broadcast_messages(messages);
            chat->callback_service->register_channel_callback(channel.channel_id(), channel_callback);
        }
#endif // PN_CHAT_C_ABI
    }
}

Channel ChannelService::create_presentation_object(String channel_id)
{
    auto chat_service_shared = chat_service.lock();

    return Channel(channel_id, chat_service_shared, shared_from_this(), chat_service_shared->presence_service, chat_service_shared->restrictions_service, 
                    chat_service_shared->message_service, chat_service_shared->membership_service);
}

ChannelEntity ChannelService::create_domain_from_presentation_data(String channel_id, ChatChannelData &presentation_data)
{
    ChannelEntity new_channel_entity;
    new_channel_entity.channel_name = presentation_data.channel_name;
    new_channel_entity.description = presentation_data.description;
    new_channel_entity.custom_data_json = presentation_data.custom_data_json;
    new_channel_entity.updated = presentation_data.updated;
    new_channel_entity.status = presentation_data.status;
    new_channel_entity.type = presentation_data.type;

    return new_channel_entity;
}

ChannelEntity ChannelService::create_domain_from_channel_response(String json_response)
{
    Json channel_response_json = Json::parse(json_response);

    if(channel_response_json.is_null())
    {
        throw std::runtime_error("can't create channel from response, response is incorrect");
    }

    Json channel_data_json = channel_response_json["data"][0];

    if(channel_data_json.is_null())
    {
        throw std::runtime_error("can't create channel from response, response doesn't have data field");
    }

    return ChannelEntity::from_json(channel_data_json);
}

ChatChannelData ChannelService::presentation_data_from_domain(ChannelEntity &channel_entity)
{
    ChatChannelData channel_data;
    channel_data.channel_name = channel_entity.channel_name;
    channel_data.description = channel_entity.description;
    channel_data.custom_data_json = channel_entity.custom_data_json;
    channel_data.updated = channel_entity.updated;
    channel_data.status = channel_entity.status;
    channel_data.type = channel_entity.type;

    return channel_data;
}

Channel ChannelService::create_channel_object(std::pair<Pubnub::String, ChannelEntity> channel_data)
{
    if (auto chat = this->chat_service.lock()) {
        this->entity_repository->get_channel_entities().update_or_insert(channel_data);

        return Channel(channel_data.first, chat, shared_from_this(), chat->presence_service, chat->restrictions_service, 
                        chat->message_service, chat->membership_service);
    } else {
        throw std::runtime_error("Chat service is not available to create channel object");
    }
}
