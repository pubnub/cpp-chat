#include "infra/pubnub.hpp"
#include <pubnub_coreapi_ex.h>
#include <vector>
extern "C" {
#include <pubnub_alloc.h>
#include <pubnub_pubsubapi.h>
#include <pubnub_ntf_sync.h>
#include <pubnub_helper.h>
#include <pubnub_objects_api.h>
#include <pubnub_blocking_io.h>
#include <pubnub_coreapi.h>
#include <pubnub_api_types.h>
#include <pubnub_subscribe_v2.h>
}

PubNub::PubNub(const Pubnub::String publish_key, const Pubnub::String subscribe_key, const Pubnub::String secret_key):
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
}

void PubNub::publish(const Pubnub::String channel, const Pubnub::String message)
{
    pubnub_publish(main_context.get(), channel.c_str(), message.c_str());

    this->await_and_handle_error();
}

void PubNub::subscribe_to_channel(const Pubnub::String channel)
{
    if (this->is_subscribed_to_channel(channel)) {
        return;
    }

    if (!subscribed_channels.empty()) {
        this->cancel_previous_subscription();
    } 

    this->subscribed_channels.push_back(channel);
    this->call_subscribe();
}

#include <iostream>

std::vector<pubnub_v2_message> PubNub::fetch_messages()
{
    auto result = pubnub_last_result(this->long_poll_context.get());
    std::cout << "Result: " << result << std::endl;
    if (PNR_OK != result && PNR_STARTED != result) {
        throw std::runtime_error(
                std::string("Failed to fetch messages: ")
                + pubnub_res_2_string(result)
                );
    }

    if (PNR_STARTED == result) {
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

void PubNub::set_channel_metadata(const Pubnub::String channel, const Pubnub::String metadata)
{
    pubnub_set_channelmetadata(
            this->main_context.get(),
            channel,
            NULL,
            metadata
    );

    this->await_and_handle_error();
}

void PubNub::await_and_handle_error()
{
    pubnub_res result = pubnub_await(main_context.get());
    if (result != PNR_OK) {
        throw std::runtime_error(pubnub_res_2_string(result));
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

    auto result = pubnub_subscribe_v2(this->long_poll_context.get(), current_channels, pubnub_subscribe_v2_defopts());
    if (PNR_OK != result && PNR_STARTED != result) {
        throw std::runtime_error(
                std::string("Failed to subscribe to channel: ")
                + pubnub_res_2_string(
                    pubnub_last_result(this->long_poll_context.get())
                    )
                );
    }
}
