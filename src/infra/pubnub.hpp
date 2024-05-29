#ifndef PN_CHAT_INFRA_PUBNUB_HPP
#define PN_CHAT_INFRA_PUBNUB_HPP

#include "string.hpp"
#include <vector>

extern "C" {
#include <pubnub_api_types.h>
#include <pubnub_helper.h>
}

class PubNub {
public:
    PubNub(const Pubnub::String publish_key, const Pubnub::String subscribe_key, const Pubnub::String user_id);
    ~PubNub() = default;

    void publish(const Pubnub::String channel, const Pubnub::String message);
    void subscribe_to_channel(const Pubnub::String channel);
    std::vector<pubnub_v2_message> fetch_messages();
    std::vector<pubnub_v2_message> pause_subscription_and_get_last_messages();
    void set_channel_metadata(const Pubnub::String channel, const Pubnub::String metadata);
    void remove_channel_metadata(const Pubnub::String channel);
    Pubnub::String get_channel_metadata(const Pubnub::String channel);
    std::vector<Pubnub::String> get_all_channels_metadata();
    void remove_members(const Pubnub::String channel, const Pubnub::String members_object);
    void set_members(const Pubnub::String channel, const Pubnub::String members_object);
    void set_user_metadata(const Pubnub::String user_id, const Pubnub::String metadata);
    Pubnub::String get_user_metadata(const Pubnub::String user_id);
    void remove_user_metadata(const Pubnub::String user_id);


private:
    void await_and_handle_error(pubnub_res result);
    bool is_subscribed_to_channel(const Pubnub::String channel);
    void cancel_previous_subscription();
    void call_subscribe();

    Pubnub::String publish_key;
    Pubnub::String subscribe_key;
    Pubnub::String user_id;

    std::unique_ptr<pubnub_t, int(*)(pubnub_t*)> main_context;
    std::unique_ptr<pubnub_t, int(*)(pubnub_t*)> long_poll_context;

    std::vector<Pubnub::String> subscribed_channels;

    bool is_subscribed = false;
};

#endif // PN_CHAT_INFRA_PUBNUB_HPP
