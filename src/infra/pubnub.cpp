#include "infra/pubnub.hpp"
#include "infra/serialization.hpp"
#include "chat.hpp"
#include "chat/message.hpp"
#include "chat/channel.hpp"
#include "chat/user.hpp"
#include "chat/membership.hpp"
#include "nlohmann/json.hpp"
#include <thread>
#include <vector>
extern "C" {
#include <pubnub_coreapi_ex.h>
#include <pubnub_alloc.h>
#include <pubnub_pubsubapi.h>
#include <pubnub_ntf_sync.h>
#include <pubnub_helper.h>
#include <pubnub_objects_api.h>
#include <pubnub_blocking_io.h>
#include <pubnub_coreapi.h>
#include <pubnub_api_types.h>
#include <pubnub_subscribe_v2.h>
#include <pubnub_fetch_history.h>
#include <pubnub_actions_api.h>
}

using json = nlohmann::json;

PubNub::PubNub(Pubnub::Chat& in_chat, const Pubnub::String publish_key, const Pubnub::String subscribe_key, const Pubnub::String secret_key):
    chat_obj(in_chat),
    publish_key(publish_key),
    subscribe_key(subscribe_key),
    user_id(secret_key),
    main_context(pubnub_alloc(), pubnub_free),
    long_poll_context(pubnub_alloc(), pubnub_free)
{
    pubnub_init(this->main_context.get(), this->publish_key.c_str(), this->subscribe_key.c_str());
    pubnub_init(this->long_poll_context.get(), this->publish_key.c_str(), this->subscribe_key.c_str());

    pubnub_set_user_id(this->main_context.get(), this->user_id.c_str());
    pubnub_set_user_id(this->long_poll_context.get(), this->user_id.c_str());

    pubnub_set_blocking_io(this->main_context.get());
    pubnub_set_non_blocking_io(this->long_poll_context.get());

    this->message_thread = std::thread([this] {
        while (!this->should_stop) {
            this->resolve_messages();
            std::this_thread::sleep_for(std::chrono::milliseconds(PUBNUB_WAIT_INTERVAL_MS));
        }
    });
}

void PubNub::publish(const Pubnub::String channel, const Pubnub::String message)
{
    auto result = pubnub_publish(main_context.get(), channel.c_str(), message.c_str());

    this->await_and_handle_error(result);
}

void PubNub::subscribe_to_channel(const Pubnub::String channel)
{
    auto messages = this->subscribe_to_channel_and_get_messages(channel);

    for (pubnub_v2_message& message : messages)
    {
        broadcast_callbacks_from_message(message);
    }
}

std::vector<pubnub_v2_message> PubNub::subscribe_to_channel_and_get_messages(const Pubnub::String channel)
{
    if (this->is_subscribed_to_channel(channel)) {
        return {};
    }

    std::vector<pubnub_v2_message> messages;

    if (this->is_subscribed) {
        messages = this->pause_subscription_and_get_messages();
    }

    this->subscribed_channels.push_back(channel);
    this->call_subscribe();

    this->is_subscribed = true;

    return messages;
}

std::vector<Pubnub::String> PubNub::subscribe_to_channel_and_get_messages_as_strings(const Pubnub::String channel)
{
    std::vector<pubnub_v2_message> pubnub_messages = this->subscribe_to_channel_and_get_messages(channel);

    std::vector<Pubnub::String> messages;
    for(pubnub_v2_message& message : pubnub_messages)
    {
        messages.push_back(Deserialization::pubnub_message_to_string(message));
    }

    return messages;
}

std::vector<pubnub_v2_message> PubNub::fetch_messages()
{
    if (!this->is_subscribed) {
        return {};
    }

    auto result = pubnub_last_result(this->long_poll_context.get());
    if (PNR_OK != result && PNR_STARTED != result && PNR_TIMEOUT != result) {
        throw std::runtime_error(
                std::string("Failed to fetch messages: ")
                + pubnub_res_2_string(result)
                );
    }

    if (PNR_STARTED == result || PNR_TIMEOUT == result) {
        if (PNR_TIMEOUT == result) {
            this->call_subscribe();
        }
        return {};
    }

    std::vector<pubnub_v2_message> messages;
    
    for (
            pubnub_v2_message message = pubnub_get_v2(this->long_poll_context.get());
            message.payload.ptr != NULL;
            message = pubnub_get_v2(this->long_poll_context.get())
        ) {
        messages.push_back(message);
   };
    
    this->call_subscribe();

    return messages;
}

std::vector<Pubnub::String> PubNub::fetch_messages_as_strings()
{
    std::vector<pubnub_v2_message> pubnub_messages = this->fetch_messages();

    std::vector<Pubnub::String> messages;
    for(pubnub_v2_message& message : pubnub_messages)
    {
        messages.push_back(Deserialization::pubnub_message_to_string(message));
    }

    return messages;
}

void PubNub::resolve_messages() {
    auto messages = this->fetch_messages();

    for (pubnub_v2_message& message : messages)
    {
        broadcast_callbacks_from_message(message);
    }
}

std::vector<pubnub_v2_message> PubNub::pause_subscription_and_get_messages()
{
    if (this->subscribed_channels.empty() || !this->is_subscribed) {
        return {};
    }

    this->cancel_previous_subscription();

    std::vector<pubnub_v2_message> messages;

    for (
            pubnub_v2_message message = pubnub_get_v2(this->long_poll_context.get());
            message.payload.ptr != NULL;
            message = pubnub_get_v2(this->long_poll_context.get())
        ) {
        messages.push_back(message);
    };

    this->is_subscribed = false;

    return messages;
}

void PubNub::unsubscribe_from_channel(Pubnub::String channel)
{
    auto messages = this->unsubscribe_from_channel_and_get_messages(channel);

    for (pubnub_v2_message& message : messages) 
    {
        broadcast_callbacks_from_message(message);
    }
}

std::vector<pubnub_v2_message> PubNub::unsubscribe_from_channel_and_get_messages(Pubnub::String channel) {
    auto messages = this->pause_subscription_and_get_messages();
    this->subscribed_channels.erase(
            std::remove(
                this->subscribed_channels.begin(),
                this->subscribed_channels.end(),
                channel
            ),
            this->subscribed_channels.end()
        );

    if (this->subscribed_channels.empty()) {
        this->is_subscribed = false;

        return messages;
    }

    this->call_subscribe();

    return messages;
}

void PubNub::resume_subscription()
{
    if (this->is_subscribed) {
        return;
    }

    this->call_subscribe();
    this->is_subscribed = true;
}

void PubNub::set_channel_metadata(const Pubnub::String channel, const Pubnub::String metadata)
{
    auto result = pubnub_set_channelmetadata(
            this->main_context.get(),
            channel,
            NULL,
            metadata
    );

    this->await_and_handle_error(result);
}

void PubNub::remove_channel_metadata(const Pubnub::String channel)
{
    auto result = pubnub_remove_channelmetadata(
            this->main_context.get(),
            channel
    );

    this->await_and_handle_error(result);
}

Pubnub::String PubNub::get_channel_metadata(const Pubnub::String channel)
{
    auto result = pubnub_get_channelmetadata(
            this->main_context.get(),
            channel,
            NULL
    );

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::get_all_channels_metadata(
        const Pubnub::String include,
        const int limit,
        const Pubnub::String start,
        const Pubnub::String end
) {
    auto result = pubnub_getall_uuidmetadata(
            this->main_context.get(),
            include,
            limit,
            start,
            end,
            pubnub_tribool::pbccFalse
    );

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::get_channel_members(
        const Pubnub::String channel,
        const Pubnub::String include,
        const int limit,
        const Pubnub::String start,
        const Pubnub::String end
) {
    auto result = pubnub_get_members(
            this->main_context.get(),
            channel,
            include,
            limit,
            start,
            end,
            pubnub_tribool::pbccFalse
    );

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

void PubNub::remove_members(const Pubnub::String channel, const Pubnub::String members_object)
{
    auto result = pubnub_remove_members(
            this->main_context.get(),
            channel,
            NULL,
            members_object
    );

    this->await_and_handle_error(result);
}

void PubNub::set_members(const Pubnub::String channel, const Pubnub::String members_object)
{
    auto result = pubnub_set_members(
            this->main_context.get(),
            channel,
            NULL,
            members_object
    );

    this->await_and_handle_error(result);
}

void PubNub::set_user_metadata(const Pubnub::String user_id, const Pubnub::String metadata)
{
    auto result = pubnub_set_uuidmetadata(
            this->main_context.get(),
            user_id,
            NULL,
            metadata
    );

    this->await_and_handle_error(result);
}

Pubnub::String PubNub::get_user_metadata(const Pubnub::String user_id)
{
    auto result = pubnub_get_uuidmetadata(
            this->main_context.get(),
            user_id,
            NULL
    );

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::get_all_user_metadata(
        const Pubnub::String include,
        const int limit,
        const Pubnub::String start,
        const Pubnub::String end
) {
    auto result = pubnub_getall_uuidmetadata(
            this->main_context.get(),
            include,
            limit,
            start,
            end,
            pubnub_tribool::pbccFalse
    );

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

void PubNub::remove_user_metadata(const Pubnub::String user_id)
{
    auto result = pubnub_remove_uuidmetadata(
            this->main_context.get(),
            user_id
    );

    this->await_and_handle_error(result);
}

Pubnub::String PubNub::get_memberships(
        const Pubnub::String user_id,
        const Pubnub::String include,
        const int limit,
        const Pubnub::String start,
        const Pubnub::String end
) {
    auto result = pubnub_get_memberships(
            this->main_context.get(),
            user_id,
            include,
            limit,
            start,
            end,
            pubnub_tribool::pbccFalse
    );

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

void PubNub::set_memberships(const Pubnub::String user_id, const Pubnub::String memberships_object)
{
    auto result = pubnub_set_memberships(
            this->main_context.get(),
            user_id,
            NULL,
            memberships_object
    );

    this->await_and_handle_error(result);
}

void PubNub::remove_memberships(const Pubnub::String user_id, const Pubnub::String memberships_object)
{
    auto result = pubnub_remove_memberships(
            this->main_context.get(),
            user_id,
            NULL,
            memberships_object
    );

    this->await_and_handle_error(result);
}

Pubnub::String PubNub::where_now(const Pubnub::String user_id)
{
    auto result = pubnub_where_now(this->main_context.get(), user_id);

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::here_now(const Pubnub::String channel)
{
    auto result = pubnub_here_now(this->main_context.get(), channel, NULL);

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::get_user_id()
{
    return this->user_id;
}

Pubnub::String PubNub::fetch_history(
        const Pubnub::String channel,
        const Pubnub::String start_timetoken,
        const Pubnub::String end_timetoken,
        const int limit
) {
    auto options = pubnub_fetch_history_defopts();
    options.max_per_channel = limit;
    options.start = start_timetoken;
    options.end = end_timetoken;
    options.include_message_actions = true;
    options.include_meta = true;

    auto result = pubnub_fetch_history(this->main_context.get(), channel, options);

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::add_message_action(const Pubnub::String channel, const Pubnub::String message_time_token, const Pubnub::pubnub_message_action_type message_action_type, const Pubnub::String value)
{
    pubnub_action_type action_type = static_cast<pubnub_action_type>(message_action_type);
    auto result = pubnub_add_message_action(this->main_context.get(), channel, message_time_token, action_type, value);

    this->await_and_handle_error(result);

    pubnub_chamebl_t add_action_response = pubnub_get_message_action_timetoken(this->main_context.get());
    
    if(!add_action_response.ptr)
    {
        return "";
    }

    return Pubnub::String(add_action_response.ptr);
}

void PubNub::register_message_callback(Pubnub::String channel_id, std::function<void(Pubnub::Message)> message_callback)
{
    this->message_callbacks_map[channel_id] = message_callback;
}

void PubNub::remove_message_callback(Pubnub::String channel_id)
{
    this->message_callbacks_map.erase(channel_id);
}

void PubNub::register_message_update_callback(Pubnub::String message_timetoken, Pubnub::String channel_id, std::function<void(Pubnub::Message)> message_update_callback)
{
    auto callback_tuple = std::make_tuple(channel_id, message_update_callback);
    this->message_update_callbacks_map[message_timetoken] = callback_tuple;
}

void PubNub::remove_message_update_callback(Pubnub::String message_timetoken)
{
    this->message_update_callbacks_map.erase(message_timetoken);
}

void PubNub::register_channel_callback(Pubnub::String channel_id, std::function<void(Pubnub::Channel)> channel_callback)
{
    this->channel_callbacks_map[channel_id] = channel_callback;
}

void PubNub::remove_channel_callback(Pubnub::String channel_id)
{
    this->channel_callbacks_map.erase(channel_id);
}

void PubNub::register_event_callback(Pubnub::String channel_id, std::function<void(Pubnub::String)> event_callback)
{
    this->event_callbacks_map[channel_id] = event_callback;
}

void PubNub::remove_event_callback(Pubnub::String channel_id)
{
    this->event_callbacks_map.erase(channel_id);
}

void PubNub::register_user_callback(Pubnub::String user_id, std::function<void(Pubnub::User)> user_callback)
{
    this->user_callbacks_map[user_id] = user_callback;
}

void PubNub::remove_user_callback(Pubnub::String user_id)
{
    this->user_callbacks_map.erase(user_id);
}

void PubNub::register_channel_presence_callback(Pubnub::String channel_id, std::function<void(std::vector<Pubnub::String>)> presence_callback)
{
    this->channel_presence_callbacks_map[channel_id] = presence_callback;
}
void PubNub::remove_channel_presence_callback(Pubnub::String channel_id)
{
    this->channel_presence_callbacks_map.erase(channel_id);
}

void PubNub::register_membership_callback(Pubnub::String channel_id, Pubnub::String user_id, std::function<void(Pubnub::Membership)> membership_callback)
{
    auto callback_tuple = std::make_tuple(user_id, membership_callback);
    this->membership_callbacks_map[channel_id] = callback_tuple;
}

void PubNub::remove_membership_callback(Pubnub::String channel_id)
{
    this->membership_callbacks_map.erase(channel_id);
}

void PubNub::stop_resolving_callbacks()
{
    this->should_stop = true;
}

void PubNub::await_and_handle_error(pubnub_res result)
{
    if (PNR_OK != result && PNR_STARTED != result) {
        throw std::runtime_error(pubnub_res_2_string(result));
    }

    if (PNR_STARTED == result) {
        pubnub_res await_result = pubnub_await(main_context.get());
        if (PNR_OK != await_result) {
            throw std::runtime_error(pubnub_res_2_string(result));
        }
    }
}

bool PubNub::is_subscribed_to_channel(const Pubnub::String channel)
{
    return std::find(
            this->subscribed_channels.begin(),
            this->subscribed_channels.end(),
            channel
    ) != this->subscribed_channels.end();
}

void PubNub::cancel_previous_subscription()
{
    if (PN_CANCEL_FINISHED != pubnub_cancel(this->long_poll_context.get())) {
        if (PNR_OK != pubnub_await(this->long_poll_context.get())) {
            throw std::runtime_error(
                    std::string("Failed to cancel previous subscription: ")
                    + pubnub_res_2_string(
                        pubnub_last_result(this->long_poll_context.get())
                    )
                );
        }
    }
    auto result = pubnub_leave(this->long_poll_context.get(), get_comma_sep_channels_to_subscribe(), NULL);
    if (PNR_OK != result) {
        if (PNR_STARTED == result && PNR_OK != pubnub_await(this->long_poll_context.get())) {
            throw std::runtime_error(
                    std::string("Failed to leave previous subscription: ")
                    + pubnub_res_2_string(
                        pubnub_last_result(this->long_poll_context.get())
                    )
                );
        }
    }
}

void PubNub::call_subscribe()
{
    auto result = pubnub_subscribe_v2(this->long_poll_context.get(), get_comma_sep_channels_to_subscribe(), pubnub_subscribe_v2_defopts());
    if (PNR_OK != result && PNR_STARTED != result) {
        throw std::runtime_error(
                std::string("Failed to subscribe to channel: ")
                + pubnub_res_2_string(result)
            );
    }
}

void PubNub::broadcast_callbacks_from_message(pubnub_v2_message message)
{
    if(!message.payload.ptr || !message.channel.ptr)
    {
        throw std::runtime_error("received message is invalid");
    }

    json message_json = json::parse(message.payload.ptr);
    
    if(message_json.is_null())
    {
        throw std::runtime_error("Failed to parse message into json");
    }

    //Handle chat messages
    if(Deserialization::is_chat_message(message.payload.ptr))
    {
        if(this->message_callbacks_map.find(message.channel.ptr) != this->message_callbacks_map.end())
        {
            this->message_callbacks_map[message.channel.ptr](
                    Deserialization::pubnub_to_chat_message(this->chat_obj, message));
        }
    }
    
    //TODO:Handle also removing channel metadata
    //Handle channel updates
    if(Deserialization::is_channel_update_message(message.payload.ptr))
    {
        if(this->channel_callbacks_map.find(message.channel.ptr) != this->channel_callbacks_map.end())
        {
            this->channel_callbacks_map[message.channel.ptr](
                    Deserialization::pubnub_message_to_chat_channel(this->chat_obj, message));
        }
    }

    //TODO:Handle also removing user metadata
    //Handle user updates
    if(Deserialization::is_user_update_message(message.payload.ptr))
    {
        if(this->user_callbacks_map.find(message.channel.ptr) != this->user_callbacks_map.end())
        {
            this->user_callbacks_map[message.channel.ptr](
                    Deserialization::pubnub_message_to_chat_user(this->chat_obj, message));
        }
    }

    //Handle events
    if(Deserialization::is_event_message(message.payload.ptr))
    {
        if(this->event_callbacks_map.find(message.channel.ptr) != this->event_callbacks_map.end())
        {
            this->event_callbacks_map[message.channel.ptr](message.payload.ptr);
        }
    }

    //Handle presence
    if(Deserialization::is_presence_message(message.payload.ptr))
    {
        if(this->channel_presence_callbacks_map.find(message.channel.ptr) != this->channel_presence_callbacks_map.end() ||
            this->channel_presence_callbacks_map.find(message.channel.ptr + Pubnub::String("-pnpres")) != this->channel_presence_callbacks_map.end())
        {
            std::vector<Pubnub::String> current_users = chat_obj.who_is_present(message.channel.ptr);
            this->channel_presence_callbacks_map[message.channel.ptr](current_users);
        }
    }

    //Handle message updates
    if(Deserialization::is_message_update_message(message.payload.ptr))
    {
        Pubnub::String message_timetoken = message_json["data"]["messageTimetoken"].dump();

        if(this->message_update_callbacks_map.find(message_timetoken) != this->message_update_callbacks_map.end())
        {
            Pubnub::String message_channel;
            std::function<void(Pubnub::Message)> callback;
            std::tie(message_channel, callback) = this->message_update_callbacks_map[message_timetoken];
            // TODO: this should already give message with this new update, make sure it really does.
            Pubnub::Message message_obj = chat_obj.get_channel(message_channel).get_message(message_timetoken);
            callback(message_obj);
        }
    }

    //Handle message updates
    if(Deserialization::is_membership_update_message(message.payload.ptr))
    {
        Pubnub::String membership_channel = message_json["data"]["messageTimetoken"]["id"].dump();

        if(this->message_update_callbacks_map.find(membership_channel) != this->message_update_callbacks_map.end())
        {
            Pubnub::String membership_user;
            std::function<void(Pubnub::Membership)> callback;
            std::tie(membership_channel, callback) = this->membership_callbacks_map[membership_channel];

            Pubnub::Membership membership_obj = Pubnub::Membership(chat_obj, chat_obj.get_channel(membership_channel), chat_obj.get_user(membership_user), Pubnub::String(message_json["custom"]));
            callback(membership_obj);
        }
    }
}

Pubnub::String PubNub::get_comma_sep_channels_to_subscribe()
{
    Pubnub::String current_channels;
    for (const auto& channel : this->subscribed_channels) {
        current_channels += channel + "," + channel + "-pnpres,";
    }
    if(!current_channels.empty())
    {
        current_channels.erase(current_channels.length() - 1, 1);
    }
    
    return current_channels;
}


