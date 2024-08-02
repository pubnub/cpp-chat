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
    // class Message;
    // class User;
    // class Channel;
    class Chat;
    // class Membership;
}
// TODO: format file
class PubNub {
public:
    PubNub(const Pubnub::String publish_key, const Pubnub::String subscribe_key, const Pubnub::String user_id);
    ~PubNub() = default;

    void publish(const Pubnub::String channel, const Pubnub::String message, const Pubnub::String metadata = "", const bool store_in_history = true, const bool send_by_post = false);
    std::vector<pubnub_v2_message> subscribe_to_channel_and_get_messages(const Pubnub::String channel);
    std::vector<pubnub_v2_message> subscribe_to_multiple_channels_and_get_messages(const std::vector<Pubnub::String> channels);
    std::vector<Pubnub::String> subscribe_to_channel_and_get_messages_as_strings(const Pubnub::String channel);
    void resolve_messages();
    std::vector<pubnub_v2_message> fetch_messages();
    std::vector<Pubnub::String> fetch_messages_as_strings();
    std::vector<pubnub_v2_message> pause_subscription_and_get_messages();
    std::vector<pubnub_v2_message> unsubscribe_from_channel_and_get_messages(Pubnub::String channel);
    void resume_subscription();
    void set_channel_metadata(const Pubnub::String channel, const Pubnub::String metadata);
    void remove_channel_metadata(const Pubnub::String channel);
    Pubnub::String get_channel_metadata(const Pubnub::String channel);
    Pubnub::String get_all_channels_metadata(const Pubnub::String include, const int limit, const Pubnub::String start, const Pubnub::String end);
    Pubnub::String get_channel_members(const Pubnub::String channel, const Pubnub::String include, const int limit, const Pubnub::String start, const Pubnub::String end);
    void remove_members(const Pubnub::String channel, const Pubnub::String members_object);
    Pubnub::String set_members(const Pubnub::String channel, const Pubnub::String members_object, const Pubnub::String include = "");
    void set_user_metadata(const Pubnub::String user_id, const Pubnub::String metadata);
    Pubnub::String get_user_metadata(const Pubnub::String user_id);
    Pubnub::String get_all_user_metadata(const Pubnub::String include, const int limit, const Pubnub::String start, const Pubnub::String end);
    void remove_user_metadata(const Pubnub::String user_id);
    Pubnub::String get_memberships(const Pubnub::String user_id, const Pubnub::String include, const int limit, const Pubnub::String start, const Pubnub::String end);
    Pubnub::String set_memberships(const Pubnub::String user_id, const Pubnub::String memberships_object, const Pubnub::String include = "");
    void remove_memberships(const Pubnub::String user_id, const Pubnub::String memberships_object);
    Pubnub::String where_now(const Pubnub::String user_id);
    Pubnub::String here_now(const Pubnub::String channel);
    Pubnub::String get_user_id();
    Pubnub::String fetch_history(const Pubnub::String channel, const Pubnub::String start_timetoken, const Pubnub::String end_timetoken, const int count);
    Pubnub::String fetch_history(const std::vector<Pubnub::String> channels, const Pubnub::String start_timetoken, const Pubnub::String end_timetoken, const int count);
    Pubnub::String add_message_action(const Pubnub::String channel, const Pubnub::String message_time_token, const Pubnub::String message_action_type, const Pubnub::String value);
    void remove_message_action(const Pubnub::String channel, const Pubnub::String message_timetoken, const Pubnub::String action_timetoken);
    std::map<Pubnub::String, int, Pubnub::StringComparer> message_counts(const std::vector<Pubnub::String> channels, const std::vector<Pubnub::String> timestamps);

    Pubnub::String parse_token(const Pubnub::String auth_key) ;

private:
    void await_and_handle_error(pubnub_res result);
    bool is_subscribed_to_channel(const Pubnub::String channel);
    void cancel_previous_subscription();
    void call_handshake();
    void call_subscribe();
    Pubnub::String get_comma_sep_channels_to_subscribe();
    Pubnub::String get_comma_sep_string_from_vector(std::vector<Pubnub::String> vector_of_strings);

    Pubnub::String publish_key;
    Pubnub::String subscribe_key;
    Pubnub::String user_id;

    std::unique_ptr<pubnub_t, int(*)(pubnub_t*)> main_context;
    std::unique_ptr<pubnub_t, int(*)(pubnub_t*)> long_poll_context;

    std::vector<Pubnub::String> subscribed_channels;

    bool is_subscribed = false;
};

#endif // PN_CHAT_INFRA_PUBNUB_HPP
