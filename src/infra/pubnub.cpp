#include "infra/pubnub.hpp"
#include "chat/message.hpp"
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

    for (Pubnub::Message& message : messages) {
        auto channel = message.get_message_data().channel_id;
        if (this->message_callbacks_map.find(channel) != this->message_callbacks_map.end()) {
            this->message_callbacks_map[channel](message);
        }
    }
}

std::vector<Pubnub::Message> PubNub::subscribe_to_channel_and_get_messages(const Pubnub::String channel)
{
    if (this->is_subscribed_to_channel(channel)) {
        return {};
    }

    std::vector<Pubnub::Message> messages;

    if (this->is_subscribed) {
        messages = this->pause_subscription_and_get_messages();
    }

    this->subscribed_channels.push_back(channel);
    this->call_subscribe();

    this->is_subscribed = true;

    return messages;
}

std::vector<Pubnub::Message> PubNub::fetch_messages()
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

    std::vector<Pubnub::Message> messages;
    
    for (
            pubnub_v2_message message = pubnub_get_v2(this->long_poll_context.get());
            message.payload.ptr != NULL;
            message = pubnub_get_v2(this->long_poll_context.get())
        ) {
        messages.push_back(this->pubnub_to_chat_message(message));
   };
    
    this->call_subscribe();

    return messages;
}

void PubNub::resolve_messages() {
    auto messages = this->fetch_messages();

    for (Pubnub::Message& message : messages) {
        auto channel = message.get_message_data().channel_id;
        if (this->message_callbacks_map.find(channel) != this->message_callbacks_map.end()) {
            this->message_callbacks_map[channel](message);
        } 
    }
}

std::vector<Pubnub::Message> PubNub::pause_subscription_and_get_messages()
{
    if (this->subscribed_channels.empty() || !this->is_subscribed) {
        return {};
    }

    this->cancel_previous_subscription();

    std::vector<Pubnub::Message> messages;

    for (
            pubnub_v2_message message = pubnub_get_v2(this->long_poll_context.get());
            message.payload.ptr != NULL;
            message = pubnub_get_v2(this->long_poll_context.get())
        ) {
        messages.push_back(this->pubnub_to_chat_message(message));
    };

    this->is_subscribed = false;

    return messages;
}

void PubNub::unsubscribe_from_channel(Pubnub::String channel)
{
    auto messages = this->unsubscribe_from_channel_and_get_messages(channel);

    for (Pubnub::Message& message : messages) {
        auto channel = message.get_message_data().channel_id;
        if (this->message_callbacks_map.find(channel) != this->message_callbacks_map.end()) {
            this->message_callbacks_map.erase(channel);
        }
    }
}

std::vector<Pubnub::Message> PubNub::unsubscribe_from_channel_and_get_messages(Pubnub::String channel) {
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

    return pubnub_get(this->main_context.get());
}

void PubNub::register_message_callback(Pubnub::String channel_id, std::function<void(Pubnub::Message)> message_callback)
{
    this->message_callbacks_map[channel_id] = message_callback;
}

void PubNub::remove_message_callback(Pubnub::String channel_id)
{
    this->message_callbacks_map.erase(channel_id);
}

void PubNub::stop_resolving_callbacks() {
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

    Pubnub::String current_channels;
    for (const auto& channel : this->subscribed_channels) {
        current_channels += channel + ",";
    }
    current_channels.erase(current_channels.length() - 1, 1);

    auto result = pubnub_leave(this->long_poll_context.get(), current_channels, NULL);
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
    Pubnub::String current_channels;
    for (const auto& channel : this->subscribed_channels) {
        current_channels += channel + ",";
    }
    current_channels.erase(current_channels.length() - 1, 1);

    auto result = pubnub_subscribe_v2(this->long_poll_context.get(), current_channels, pubnub_subscribe_v2_defopts());
    if (PNR_OK != result && PNR_STARTED != result) {
        throw std::runtime_error(
                std::string("Failed to subscribe to channel: ")
                + pubnub_res_2_string(result)
            );
    }
}

Pubnub::Message PubNub::pubnub_to_chat_message(pubnub_v2_message pn_message)
{
    // TODO: implement message parsing properly
    auto to_pn_string = [](struct pubnub_char_mem_block message) {
        return Pubnub::String(message.ptr, message.size);
    };

    return Pubnub::Message(
            chat_obj,
            to_pn_string(pn_message.tt),
            Pubnub::ChatMessageData{
                Pubnub::pubnub_chat_message_type::PCMT_TEXT,
                to_pn_string(pn_message.payload),
                to_pn_string(pn_message.channel),
                to_pn_string(pn_message.publisher),
                to_pn_string(pn_message.metadata),
                {}
            }
        );
}
