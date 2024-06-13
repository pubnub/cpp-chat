#ifndef PN_CHAT_INFRA_PUBNUB_HPP
#define PN_CHAT_INFRA_PUBNUB_HPP

#include "string.hpp"
#include "enums.hpp"
#include <thread>
#include <vector>
#include <map>
#include <tuple>
#include <functional>

extern "C" {
#include <pubnub_api_types.h>
#include <pubnub_helper.h>
}

#ifndef PUBNUB_WAIT_INTERVAL_MS
#define PUBNUB_WAIT_INTERVAL_MS 100
#endif

namespace Pubnub
{
    class Message;
    class User;
    class Channel;
    class Chat;
    class Membership;
}
// TODO: format file
class PubNub {
public:
    PubNub(Pubnub::Chat& in_chat, const Pubnub::String publish_key, const Pubnub::String subscribe_key, const Pubnub::String user_id);
    ~PubNub(){
        // TODO: synchronization might fail because of lack of any mutexes
        this->should_stop = true;
        if (this->message_thread.joinable()) {
            this->message_thread.join();
        }
    };

    void publish(const Pubnub::String channel, const Pubnub::String message);
    void subscribe_to_channel(const Pubnub::String channel);
    std::vector<pubnub_v2_message> subscribe_to_channel_and_get_messages(const Pubnub::String channel);
    std::vector<Pubnub::String> subscribe_to_channel_and_get_messages_as_strings(const Pubnub::String channel);
    void resolve_messages();
    std::vector<pubnub_v2_message> fetch_messages();
    std::vector<Pubnub::String> fetch_messages_as_strings();
    std::vector<pubnub_v2_message> pause_subscription_and_get_messages();
    void unsubscribe_from_channel(Pubnub::String channel);
    std::vector<pubnub_v2_message> unsubscribe_from_channel_and_get_messages(Pubnub::String channel);
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
    Pubnub::String add_message_action(const Pubnub::String channel, const Pubnub::String message_time_token, const Pubnub::pubnub_message_action_type message_action_type, const Pubnub::String value);

    /* CALLBACKS */
    void register_message_callback(Pubnub::String channel_id, std::function<void(Pubnub::Message)> message_callback);
    void remove_message_callback(Pubnub::String channel_id);
    void register_message_update_callback(Pubnub::String message_timetoken, Pubnub::String channel_id, std::function<void(Pubnub::Message)> message_update_callback);
    void remove_message_update_callback(Pubnub::String message_timetoken);
    void register_channel_callback(Pubnub::String channel_id, std::function<void(Pubnub::Channel)> channel_callback);
    void remove_channel_callback(Pubnub::String channel_id);
    void register_event_callback(Pubnub::String channel_id, std::function<void(Pubnub::String)> event_callback);
    void remove_event_callback(Pubnub::String channel_id);
    void register_user_callback(Pubnub::String user_id, std::function<void(Pubnub::User)> user_callback);
    void remove_user_callback(Pubnub::String user_id);
    void register_channel_presence_callback(Pubnub::String channel_id, std::function<void(std::vector<Pubnub::String>)> presence_callback);
    void remove_channel_presence_callback(Pubnub::String channel_id);
    void register_membership_callback(Pubnub::String channel_id, Pubnub::String user_id, std::function<void(Pubnub::Membership)> membership_callback);
    void remove_membership_callback(Pubnub::String channel_id);

    // TODO: not the greatest way but just for mvp...
    void stop_resolving_callbacks();

private:
    void await_and_handle_error(pubnub_res result);
    bool is_subscribed_to_channel(const Pubnub::String channel);
    void cancel_previous_subscription();
    void call_subscribe();
    Pubnub::String get_comma_sep_channels_to_subscribe();
    void broadcast_callbacks_from_message(pubnub_v2_message message);

    Pubnub::String publish_key;
    Pubnub::String subscribe_key;
    Pubnub::String user_id;
    Pubnub::Chat& chat_obj;

    std::unique_ptr<pubnub_t, int(*)(pubnub_t*)> main_context;
    std::unique_ptr<pubnub_t, int(*)(pubnub_t*)> long_poll_context;

    std::vector<Pubnub::String> subscribed_channels;
    std::map<Pubnub::String, std::function<void(Pubnub::Message)>, Pubnub::StringComparer> message_callbacks_map;
    std::map<Pubnub::String, std::tuple<Pubnub::String, std::function<void(Pubnub::Message)>>, Pubnub::StringComparer> message_update_callbacks_map; 
    std::map<Pubnub::String, std::function<void(Pubnub::Channel)>, Pubnub::StringComparer> channel_callbacks_map;
    std::map<Pubnub::String, std::function<void(Pubnub::String)>, Pubnub::StringComparer> event_callbacks_map;
    std::map<Pubnub::String, std::function<void(Pubnub::User)>, Pubnub::StringComparer> user_callbacks_map;
    std::map<Pubnub::String, std::function<void(std::vector<Pubnub::String>)>, Pubnub::StringComparer> channel_presence_callbacks_map;
    std::map<Pubnub::String, std::tuple<Pubnub::String, std::function<void(Pubnub::Membership)>>, Pubnub::StringComparer> membership_callbacks_map; 



    bool is_subscribed = false;
    bool should_stop = false;

    std::thread message_thread;
};

#endif // PN_CHAT_INFRA_PUBNUB_HPP
