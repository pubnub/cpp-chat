#ifndef PN_CHAT_INFRA_PUBNUB_HPP
#define PN_CHAT_INFRA_PUBNUB_HPP

#include "application/subscription.hpp"
#include "string.hpp"
#include "enums.hpp"
#include "logger.hpp"
#include <memory>
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
    class Chat;
    struct ConnectionStatusData;
}
// TODO: format file
class PubNub {
public:
    PubNub(const Pubnub::String publish_key, const Pubnub::String subscribe_key, const Pubnub::String user_id, const Pubnub::String auth_key);
    ~PubNub();

    Pubnub::String publish(const Pubnub::String channel, const Pubnub::String message, const Pubnub::String metadata = "", const bool store_in_history = true, const bool send_by_post = false);
    Pubnub::String signal(const Pubnub::String channel, const Pubnub::String message);
    std::shared_ptr<Subscription> subscribe(const Pubnub::String& channel);
    std::shared_ptr<SubscriptionSet> subscribe_multiple(const std::vector<Pubnub::String>& channels);
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
    Pubnub::String get_all_channels_metadata(const Pubnub::String include, const int limit, const Pubnub::String filter, const Pubnub::String sort, const Pubnub::String page_next, const Pubnub::String page_prev);
    Pubnub::String get_channel_members(const Pubnub::String channel, const Pubnub::String include, const int limit, const Pubnub::String filter, const Pubnub::String sort, const Pubnub::String page_next, const Pubnub::String page_prev);
    void remove_members(const Pubnub::String channel, const Pubnub::String members_object);
    Pubnub::String set_members(const Pubnub::String channel, const Pubnub::String members_object, const Pubnub::String include = "", const Pubnub::String filter = "");
    void set_user_metadata(const Pubnub::String user_id, const Pubnub::String metadata);
    Pubnub::String get_user_metadata(const Pubnub::String user_id);
    Pubnub::String get_all_user_metadata(const Pubnub::String include, const int limit, const Pubnub::String filter, const Pubnub::String sort, const Pubnub::String page_next, const Pubnub::String page_prev);
    void remove_user_metadata(const Pubnub::String user_id);
    Pubnub::String get_memberships(const Pubnub::String user_id, const Pubnub::String include, const int limit, const Pubnub::String filter, const Pubnub::String sort, const Pubnub::String page_next, const Pubnub::String page_prev);
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
    bool delete_messages(const Pubnub::String channel, const Pubnub::String start, const Pubnub::String end);
    
    Pubnub::String parse_token(const Pubnub::String auth_key);
    Pubnub::String get_current_auth_token();
    void set_auth_token(const Pubnub::String token);
    int set_pubnub_origin(const Pubnub::String origin);

    void set_logging_callback(void (*callback)(enum pubnub_log_level log_level, const char* message));

    void add_connection_status_listener(std::function<void(Pubnub::pn_connection_status status, Pubnub::ConnectionStatusData status_data)> listener);
    void remove_connection_status_listener();
    bool reconnect_subscriptions();
    bool disconnect_subscriptions();

private:
    void await_and_handle_error(pubnub_res result);
    bool is_subscribed_to_channel(const Pubnub::String channel);
    void cancel_previous_subscription();
    void call_handshake();
    void call_subscribe();
    Pubnub::String get_comma_sep_channels_to_subscribe();
    Pubnub::String get_comma_sep_string_from_vector(std::vector<Pubnub::String> vector_of_strings);
    static Pubnub::pn_connection_status pn_subscription_status_to_connection_status(const pubnub_subscription_status subscription_status);

    Pubnub::String publish_key;
    Pubnub::String subscribe_key;
    Pubnub::String user_id;
    Pubnub::String auth_key;
    Pubnub::String custom_origin;

    std::unique_ptr<pubnub_t, int(*)(pubnub_t*)> main_context;
    std::unique_ptr<pubnub_t, int(*)(pubnub_t*)> long_poll_context;

    std::vector<Pubnub::String> subscribed_channels;
    std::function<void(Pubnub::pn_connection_status, Pubnub::ConnectionStatusData)> status_listener;
    pubnub_subscribe_status_callback_t status_listener_callback;
    bool status_listener_added = false;

    bool is_subscribed = false;
};

#endif // PN_CHAT_INFRA_PUBNUB_HPP
