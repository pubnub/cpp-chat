#ifndef PN_CHAT_INFRA_PUBNUB_HPP
#define PN_CHAT_INFRA_PUBNUB_HPP

#include "string.hpp"
#include "chat/message.hpp"
#include <thread>
#include <vector>
#include <map>
#include <functional>

extern "C" {
#include <pubnub_api_types.h>
#include <pubnub_helper.h>
}

#ifndef PUBNUB_WAIT_INTERVAL_MS
#define PUBNUB_WAIT_INTERVAL_MS 100
#endif

// TODO: format file
class PubNub {
public:
    PubNub(Pubnub::Chat& in_chat, const Pubnub::String publish_key, const Pubnub::String subscribe_key, const Pubnub::String user_id);
    ~PubNub(){
        // TODO: synchronization might fail because of lack of any mutexes
        this->should_stop = true;
        this->message_thread.join();
    };

    void publish(const Pubnub::String channel, const Pubnub::String message);
    void subscribe_to_channel(const Pubnub::String channel);
    void resolve_messages();
    void pause_subscription_and_resolve_messages();
    void unsubscribe_from_channel(Pubnub::String channel);
    void resume_subscription();
    void set_channel_metadata(const Pubnub::String channel, const Pubnub::String metadata);
    void remove_channel_metadata(const Pubnub::String channel);
    Pubnub::String get_channel_metadata(const Pubnub::String channel);
    Pubnub::String get_all_channels_metadata(const Pubnub::String include, const int limit, const Pubnub::String start, const Pubnub::String end);
    Pubnub::String get_channel_members(const Pubnub::String channel, const Pubnub::String include, const int limit, const Pubnub::String start, const Pubnub::String end);
    void remove_members(const Pubnub::String channel, const Pubnub::String members_object);
    void set_members(const Pubnub::String channel, const Pubnub::String members_object);
    void set_user_metadata(const Pubnub::String user_id, const Pubnub::String metadata);
    Pubnub::String get_user_metadata(const Pubnub::String user_id);
    Pubnub::String get_all_user_metadata(const Pubnub::String include, const int limit, const Pubnub::String start, const Pubnub::String end);
    void remove_user_metadata(const Pubnub::String user_id);
    Pubnub::String get_memberships(const Pubnub::String user_id, const Pubnub::String include, const int limit, const Pubnub::String start, const Pubnub::String end);
    void set_memberships(const Pubnub::String user_id, const Pubnub::String memberships_object);
    void remove_memberships(const Pubnub::String user_id, const Pubnub::String memberships_object);
    Pubnub::String where_now(const Pubnub::String user_id);
    Pubnub::String here_now(const Pubnub::String channel);
    Pubnub::String get_user_id();
    Pubnub::String fetch_history(const Pubnub::String channel, const Pubnub::String start_timetoken, const Pubnub::String end_timetoken, const int count);


    void register_message_callback(Pubnub::String channel_id, std::function<void(Pubnub::Message)> message_callback);
    void remove_message_callback(Pubnub::String channel_id);

private:
    void await_and_handle_error(pubnub_res result);
    bool is_subscribed_to_channel(const Pubnub::String channel);
    void cancel_previous_subscription();
    void call_subscribe();
    Pubnub::Message pubnub_to_chat_message(pubnub_v2_message pn_message);

    Pubnub::String publish_key;
    Pubnub::String subscribe_key;
    Pubnub::String user_id;
    Pubnub::Chat& chat_obj;

    std::unique_ptr<pubnub_t, int(*)(pubnub_t*)> main_context;
    std::unique_ptr<pubnub_t, int(*)(pubnub_t*)> long_poll_context;

    std::vector<Pubnub::String> subscribed_channels;
    std::map<Pubnub::String, std::function<void(Pubnub::Message)>, Pubnub::StringComparer> message_callbacks_map;

    bool is_subscribed = false;
    bool should_stop = false;

    std::thread message_thread;
};

#endif // PN_CHAT_INFRA_PUBNUB_HPP
