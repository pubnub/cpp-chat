#ifndef PN_CHAT_CALLBACK_SERVICE_HPP
#define PN_CHAT_CALLBACK_SERVICE_HPP

extern "C" {
#include <pubnub_subscribe_event_listener_types.h>
}
#include "application/bundles.hpp"
#include "application/channel_service.hpp"
#include "application/chat_service.hpp"
#include "application/message_service.hpp"
#include "application/presence_service.hpp"
#include "channel.hpp"
#include "infra/callbacks_repository.hpp"
#include "infra/entity_repository.hpp"
#include "infra/pubnub.hpp"
#include "infra/sync.hpp"
#include "membership.hpp"
#include <any>
#include <chrono>
#include <list>
#include <memory>
#include <thread>
#include <atomic>

#ifndef PUBNUB_WAIT_INTERVAL_MS
#define PUBNUB_WAIT_INTERVAL_MS 100
#endif

struct CCoreCallbackData {
    pubnub_subscribe_message_callback_t callback;
    std::any& context;
};

class CallbackService {
    using milliseconds = std::chrono::milliseconds;

    public:
        CallbackService(
                EntityServicesBundle entity_bundle,
                std::weak_ptr<const PresenceService> channel_service,
                ThreadSafePtr<PubNub> pubnub,
                milliseconds wait_interval = milliseconds(PUBNUB_WAIT_INTERVAL_MS)
        );
        ~CallbackService() {
            this->thread_run_flag.store(false);

            if (this->callback_thread.joinable()) {
                this->callback_thread.join();
            }
        };

        void register_message_callback(Pubnub::String channel_id, std::function<void(Pubnub::Message)> message_callback);
        void remove_message_callback(Pubnub::String channel_id);

        void register_message_update_callback(
                Pubnub::String message_timetoken,
                std::function<void(Pubnub::Message)> message_update_callback
        );
        void remove_message_update_callback(Pubnub::String message_timetoken);

        void register_thread_message_update_callback(Pubnub::String channel_id, std::function<void(Pubnub::ThreadMessage)> thread_message_callback);
        void remove_thread_message_update_callback(Pubnub::String channel_id);

        void register_channel_callback(Pubnub::String channel_id, std::function<void(Pubnub::Channel)> channel_callback);
        void remove_channel_callback(Pubnub::String channel_id);

        void register_event_callback(
                Pubnub::String channel_id,
                Pubnub::pubnub_chat_event_type chat_event_type,
                std::function<void(Pubnub::Event)> event_callback
        );
        void remove_event_callback(Pubnub::String channel_id, Pubnub::pubnub_chat_event_type chat_event_type);

        void register_user_callback(Pubnub::String user_id, std::function<void(Pubnub::User)> user_callback);
        void remove_user_callback(Pubnub::String user_id);

        void register_channel_presence_callback(Pubnub::String channel_id, std::function<void(std::vector<Pubnub::String>)> presence_callback);
        void remove_channel_presence_callback(Pubnub::String channel_id);

        void register_membership_callback(
                Pubnub::String channel_id,
                Pubnub::String user_id,
                std::function<void(Pubnub::Membership)> membership_callback
        );
        void remove_membership_callback(Pubnub::String channel_id);

        void broadcast_messages(std::vector<pubnub_v2_message> messages);

        void add_connection_status_listener(std::function<void(Pubnub::pn_connection_status status, Pubnub::ConnectionStatusData status_data)> listener);
        void remove_connection_status_listener();

        CCoreCallbackData to_c_message_callback(std::weak_ptr<const ChatService> chat_service, std::function<void(Pubnub::Message)> message_callback);
        CCoreCallbackData to_c_channel_update_callback(Pubnub::Channel channel, std::shared_ptr<const ChannelService> chat_service, std::function<void(Pubnub::Channel)> channel_update_callback);
        CCoreCallbackData to_c_channels_updates_callback(const std::vector<Pubnub::Channel>& channels, std::shared_ptr<const ChannelService> chat_service, std::function<void(std::vector<Pubnub::Channel>)> channel_update_callback);
        CCoreCallbackData to_c_user_update_callback(Pubnub::User user_base, std::shared_ptr<const UserService> user_service, std::function<void (Pubnub::User)> user_update_callback);
        CCoreCallbackData to_c_users_updates_callback(const std::vector<Pubnub::User>& users, std::shared_ptr<const UserService> user_service, std::function<void(std::vector<Pubnub::User>)> user_update_callback);
        CCoreCallbackData to_c_event_callback(Pubnub::pubnub_chat_event_type chat_event_type, std::function<void(Pubnub::Event)> event_callback);
        CCoreCallbackData to_c_presence_callback(Pubnub::String channel_id, std::shared_ptr<const PresenceService> presence_service, std::function<void(std::vector<Pubnub::String>)> presence_callback);
        CCoreCallbackData to_c_membership_update_callback(Pubnub::Membership membership, std::weak_ptr<const ChatService> chat_service, std::function<void(Pubnub::Membership)> membership_callback);
        CCoreCallbackData to_c_memberships_updates_callback(const std::vector<Pubnub::Membership>& memberships, std::weak_ptr<const ChatService> chat_service, std::function<void(std::vector<Pubnub::Membership>)> membership_callback);
        CCoreCallbackData to_c_message_update_callback(Pubnub::Message base_message, std::shared_ptr<const MessageService> message_service, std::function<void(Pubnub::Message)> message_update_callback);
        CCoreCallbackData to_c_thread_message_update_callback(Pubnub::ThreadMessage base_message, std::shared_ptr<const MessageService> message_service, std::function<void(Pubnub::ThreadMessage)> message_update_callback);
        CCoreCallbackData to_c_messages_updates_callback(const std::vector<Pubnub::Message>& messages, std::shared_ptr<const MessageService> message_service, std::function<void(std::vector<Pubnub::Message>)> message_update_callback);
        CCoreCallbackData to_c_thread_messages_updates_callback(const std::vector<Pubnub::ThreadMessage>& messages, std::shared_ptr<const MessageService> message_service, std::function<void(std::vector<Pubnub::ThreadMessage>)> message_update_callback);
    private:
        void resolve_callbacks();
        void resolve_timers(milliseconds wait_interval);
        void broadcast_callbacks_from_message(pubnub_v2_message message);

        CallbacksRepository callbacks;
        ThreadSafePtr<PubNub> pubnub;
        std::weak_ptr<const ChannelService> channel_service;
        std::weak_ptr<const UserService> user_service;
        std::shared_ptr<const ChatService> chat_service;
        std::weak_ptr<const MessageService> message_service;
        std::weak_ptr<const MembershipService> membership_service;
        std::weak_ptr<const PresenceService> presence_service;
        std::thread callback_thread;
        std::atomic<bool> thread_run_flag;
        std::list<std::any> callback_contexts;

        std::function<void(Pubnub::pn_connection_status, Pubnub::ConnectionStatusData)> status_listener;
        pubnub_subscribe_status_callback_t status_listener_callback;
        bool status_listener_added = false;
};

#endif // PN_CHAT_CALLBACK_SERVICE_HPP
