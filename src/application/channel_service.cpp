#include "channel_service.hpp"
#include <pubnub_subscribe_event_listener_types.h>
#include "application/subscription.hpp"
#include "event.hpp"
#include "callback_stop.hpp"
#include "enum_converters.hpp"
#include "infra/rate_limiter.hpp"
#include "thread_channel.hpp"
#include "thread_message.hpp"
#include "application/dao/channel_dao.hpp"
#include "application/dao/membership_dao.hpp"
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
#include <algorithm>
#include <functional>
#include <memory>
#ifdef PN_CHAT_C_ABI
#include <pubnub_helper.h>
#include "domain/parsers.hpp"
#endif // PN_CHAT_C_ABI

using namespace Pubnub;
using json = nlohmann::json;

ChannelService::ChannelService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service, float exponential_factor) :
    pubnub(pubnub),
    chat_service(chat_service),
    rate_limiter(ExponentialRateLimiter(exponential_factor))
{}

Channel ChannelService::create_public_conversation(const String& channel_id, const ChannelDAO& data) const {
    auto new_entity = data.to_entity();
    new_entity.type = "public";
    return create_channel(channel_id, std::move(new_entity));
}

std::tuple<Channel, Membership, std::vector<Membership>> ChannelService::create_direct_conversation(const User& user, const String& channel_id, const ChannelDAO& channel_data, const Pubnub::ChatMembershipData& membership_data) const {
    //TODO: channel id should be optional and if it's not provided, we should create hashed channel id
    String final_channel_id = channel_id;

    auto new_entity = channel_data.to_entity();
    new_entity.type = "direct";
    auto created_channel = this->create_channel(final_channel_id, std::move(new_entity));
    String user_id;

    {
        auto pubnub_handle = this->pubnub->lock();

        //TODO: Add filter when it will be supported in C-Core
        String include_string = "custom,channel,totalCount,channel.custom";
        user_id = pubnub_handle->get_user_id();
        pubnub_handle->set_memberships(pubnub_handle->get_user_id(), create_set_memberships_object(final_channel_id), include_string);
    }

    auto chat_service_shared = chat_service.lock();

    //TODO: Maybe current user should just be created in chat constructor and stored there all the time?
    User current_user = chat_service_shared->user_service->get_user(user_id);

    MembershipEntity host_membership_entity = MembershipDAO(membership_data).to_entity();
    Membership host_membership = chat_service_shared->membership_service->create_membership_object(current_user, created_channel, host_membership_entity);
    Membership invitee_membership = chat_service_shared->membership_service->invite_to_channel(final_channel_id, *created_channel.data, user);

    return std::make_tuple(created_channel, host_membership, std::vector<Membership>{invitee_membership});
}

std::tuple<Channel, Membership, std::vector<Membership>> ChannelService::create_group_conversation(const std::vector<User>& users, const String& channel_id, const ChannelDAO& channel_data, const Pubnub::ChatMembershipData& membership_data) const {
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
        String include_string = "custom,channel,totalCount,custom.channel";
        String memberships_response = pubnub_handle->set_memberships(user_id, create_set_memberships_object(final_channel_id), include_string);
    }

    auto chat_service_shared = chat_service.lock();

    //TODO: Maybe current user should just be created in chat constructor and stored there all the time?
    User current_user = chat_service_shared->user_service->get_user(user_id);

    MembershipEntity host_membership_entity = MembershipDAO(membership_data).to_entity();
    Membership host_membership = chat_service_shared->membership_service->create_membership_object(current_user, created_channel, host_membership_entity);
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
    if(limit < 0 || limit > PN_MAX_LIMIT)
    {
        throw std::invalid_argument("can't get channels, limit has to be within 0 - " + std::to_string(PN_MAX_LIMIT) + " range");
    }
    
    Pubnub::String include = "custom,totalCount,channel";
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

std::shared_ptr<Subscription> ChannelService::connect(const String& channel_id, const ChannelDAO& channel_data, std::function<void(Message)> message_callback) const {
    auto subscription = this->pubnub->lock()->subscribe(channel_id);

    channel_data.add_chat_message_listener(subscription);

    auto chat_service_shared = this->chat_service.lock();
    subscription->add_message_listener(chat_service_shared->callback_service->to_c_message_callback(this->chat_service, message_callback));

    return subscription;
}

void ChannelService::disconnect(const ChannelDAO& channel_data) const {
      channel_data.stop_listening_for_chat_messages();
}

std::shared_ptr<Subscription> ChannelService::join(const Channel& channel, const ChannelDAO& channel_data, std::function<void(Message)> message_callback, const ChatMembershipData& membership_data) const {
   MembershipEntity membership_entity = MembershipDAO(membership_data).to_entity();
   String set_object_string = membership_entity.get_set_memberships_json_string(channel.channel_id());

    auto memberships_response = [this, set_object_string] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->set_memberships(pubnub_handle->get_user_id(), set_object_string);
    }();

    auto chat_service_shared = chat_service.lock();

    auto user = chat_service_shared->user_service->get_current_user();
    
    auto membership = chat_service_shared->membership_service->create_membership_object(user, channel, membership_entity);

    auto new_membership = membership.set_last_read_message_timetoken(Pubnub::get_now_timetoken());

    return this->connect(channel.channel_id(), channel_data, message_callback);
}
    
void ChannelService::leave(const String& channel_id, const ChannelDAO& channel_data) const {
    String remove_object_string = String("[{\"channel\": {\"id\": \"") + channel_id + String("\"}}]");

	this->disconnect(channel_data);

    auto pubnub_handle = this->pubnub->lock();
    String user_id = pubnub_handle->get_user_id();
    pubnub_handle->remove_memberships(user_id, remove_object_string);
}

void ChannelService::send_text(const Pubnub::String& channel_id, const ChannelDAO& dao, const Pubnub::String &message, const SendTextParamsInternal& text_params) const
{
    if(!text_params.quoted_message.timetoken.empty() && text_params.quoted_message.channel_id != channel_id)
    {
        throw std::invalid_argument("You cannot quote messages from other channels");
    }

    const auto channel_type = dao.get_entity().type;
    const auto base_intervals = this->chat_service.lock()->chat_config.rate_limit_per_channel;

    const auto base_interval_ms = channel_type == Pubnub::String("public") 
        ? base_intervals.public_conversation : channel_type == Pubnub::String("direct") 
        ? base_intervals.direct_conversation : channel_type == Pubnub::String("group") 
        ? base_intervals.group_conversation : base_intervals.unknown_conversation;

    this->rate_limiter.run_within_limits(
        channel_id,
        base_interval_ms,
        [this, channel_id, message, text_params]() {
            return this->pubnub
                ->lock()
                ->publish(
                        channel_id,
                        chat_message_to_publish_string(message, pubnub_chat_message_type::PCMT_TEXT),
                        this->send_text_meta_from_params(text_params),
                        text_params.store_in_history,
                        text_params.send_by_post
                );
        },
        [this, text_params, message, channel_id](Pubnub::String mention_timetoken) {
            if(text_params.mentioned_users.size() > 0)
            {
                for(auto it = text_params.mentioned_users.begin(); it != text_params.mentioned_users.end(); it++) {
                    this->emit_user_mention(channel_id, it->second.id, mention_timetoken, message);
                }
            }
        },
        [](std::exception& e) { throw e; }
    );
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

    channel_data.start_typing(chat_service_shared->chat_config.typing_timeout - chat_service_shared->chat_config.typing_timeout_difference);

    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_TYPING, channel_id, Typing::payload(true));
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

    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_TYPING, channel_id, Typing::payload(false));
}

std::shared_ptr<Subscription> ChannelService::get_typing(const String& channel_id, ChannelDAO& channel_data, std::function<void(const std::vector<String>&)> typing_callback) const {
    auto chat_service_shared = chat_service.lock();
    auto typing_timeout = chat_service_shared->chat_config.typing_timeout;
    std::function<void(Event)> internal_typing_callback = [&channel_data, typing_callback, typing_timeout] (Event event)
    {
        auto maybe_typing = Typing::typing_value_from_event(event);
        if(!maybe_typing.has_value()) {
            throw std::runtime_error("Can't get typing from payload");
        }

        auto user_id = event.user_id;
        auto typing_value = maybe_typing.value();
       
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

    return chat_service_shared->listen_for_events(channel_id, pubnub_chat_event_type::PCET_TYPING, internal_typing_callback);
}

Message ChannelService::get_pinned_message(const String& channel_id, const ChannelDAO& channel_data) const {
    if (channel_data.get_entity().custom_data_json.empty()) {
        throw std::invalid_argument("there is no any pinned message");
    }

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
    if(limit < 0 || limit > PN_MAX_LIMIT)
    {
        throw std::invalid_argument("can't get channel suggestions, limit has to be within 0 - " + std::to_string(PN_MAX_LIMIT) + " range");
    }

    auto chat_shared = this->chat_service.lock();

    if(!chat_shared)
    {
        throw std::runtime_error("can't get channel suggestions, chat service is invalid");
    }

    String cache_key = chat_shared->message_service->get_channel_phrase_to_look_for(text);

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
    if(limit < 0 || limit > PN_MAX_LIMIT)
    {
        throw std::invalid_argument("can't get users suggestions, limit has to be within 0 - " + std::to_string(PN_MAX_LIMIT) + " range");
    }

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

std::shared_ptr<Subscription> ChannelService::stream_updates(Pubnub::Channel calling_channel, std::function<void(Channel)> channel_callback) const
{
    auto pubnub_handle = this->pubnub->lock();

    auto subscription = pubnub_handle->subscribe(calling_channel.channel_id());

    auto chat_service_shared = this->chat_service.lock();
    subscription->add_channel_update_listener(
            chat_service_shared->callback_service->to_c_channel_update_callback(calling_channel, shared_from_this(), channel_callback));

    return subscription;
}

std::shared_ptr<SubscriptionSet> ChannelService::stream_updates_on(Pubnub::Channel calling_channel, const std::vector<Pubnub::Channel>& channels, std::function<void(std::vector<Channel>)> channel_callback) const
{
    if(channels.empty())
    {
        throw std::invalid_argument("Cannot stream channel updates on an empty list");
    }

    std::vector<Pubnub::String> channels_ids;

    std::transform(
            channels.begin(),
            channels.end(),
            std::back_inserter(channels_ids),
            [](const Pubnub::Channel& channel) {
                return channel.channel_id();
            });

    auto subscription = this->pubnub->lock()->subscribe_multiple(channels_ids);

    auto chat_service_shared = this->chat_service.lock();
    subscription->add_channel_update_listener(
            chat_service_shared->callback_service->to_c_channels_updates_callback(channels, shared_from_this(), channel_callback));

    return subscription;
}


std::shared_ptr<Subscription> ChannelService::stream_read_receipts(const Pubnub::String& channel_id, const ChannelDAO& channel_data, std::function<void(std::map<Pubnub::String, std::vector<Pubnub::String>, Pubnub::StringComparer>)> read_receipts_callback) const
{
    if(channel_data.get_entity().type == String("public"))
    {
        //throw std::runtime_error("Read receipts are not supported in Public chats");
    }

    auto chat_service_shared = this->chat_service.lock();

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

    auto receipt_event_callback = [=, &channel_data](const Pubnub::Event& event){

        std::map<String, String, StringComparer> timetoken_per_user_in;

        auto members_tuple2 = chat_service_shared->membership_service->get_channel_members(channel_id, channel_data);
        auto channel_members2 = std::get<0>(members_tuple);
        for(auto membership : channel_members2)
        {
            String last_read_timetoken = membership.last_read_message_timetoken();
            if(last_read_timetoken.empty())
            {
                continue;
            }

            timetoken_per_user_in[membership.user.user_id()] =  last_read_timetoken;
            
        }

        Json payload_json = Json::parse(event.payload);

        timetoken_per_user_in[event.user_id] = payload_json.get_string("messageTimetoken").value_or(String(""));

        read_receipts_callback(generate_receipts(timetoken_per_user_in));
    };

    return chat_service_shared->listen_for_events(channel_id, pubnub_chat_event_type::PCET_RECEPIT, receipt_event_callback);
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

std::tuple<std::vector<Pubnub::Event>, bool> ChannelService::get_message_reports_history(const Pubnub::String& channel_id, const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, int count) const {
    if (auto chat_service = this->chat_service.lock()) {
        const auto channel = INTERNAL_MODERATION_PREFIX + channel_id;

        return chat_service->get_events_history(channel, start_timetoken, end_timetoken, count);
    } else {
        throw std::runtime_error("Chat service is not available to get message reports history");
    }
}

std::shared_ptr<Subscription> ChannelService::stream_message_reports(const Pubnub::String& channel_id, std::function<void(Pubnub::Event)> message_report_callback) const {
    if (auto chat_service = this->chat_service.lock()) {
        const auto channel = INTERNAL_MODERATION_PREFIX + channel_id;

        return chat_service->listen_for_events(channel, Pubnub::PCET_REPORT, message_report_callback);
    } else {
        throw std::runtime_error("Chat service is not available to stream message reports");
    }
}

