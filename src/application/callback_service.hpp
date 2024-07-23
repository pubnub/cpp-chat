#ifndef PN_CHAT_CALLBACK_SERVICE_HPP
#define PN_CHAT_CALLBACK_SERVICE_HPP

#include "application/bundles.hpp"
#include "application/presence_service.hpp"
#include "infra/callbacks_repository.hpp"
#include "infra/entity_repository.hpp"
#include "infra/pubnub.hpp"
#include "infra/sync.hpp"
#include <chrono>
#include <thread>
#include <atomic>

#ifndef PUBNUB_WAIT_INTERVAL_MS
#define PUBNUB_WAIT_INTERVAL_MS 100
#endif

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
                Pubnub::String channel_id,
                std::function<void(Pubnub::Message)> message_update_callback
        );
        void remove_message_update_callback(Pubnub::String message_timetoken);

        void register_channel_callback(Pubnub::String channel_id, std::function<void(Pubnub::Channel)> channel_callback);
        void remove_channel_callback(Pubnub::String channel_id);

        void register_event_callback(
                Pubnub::String channel_id,
                Pubnub::pubnub_chat_event_type chat_event_type,
                std::function<void(Pubnub::String)> event_callback
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
    private:
        void resolve_callbacks();
        void resolve_timers(milliseconds wait_interval);
        void broadcast_callbacks_from_message(pubnub_v2_message message);

        CallbacksRepository callbacks;
        ThreadSafePtr<PubNub> pubnub;
        std::weak_ptr<const ChannelService> channel_service;
        std::weak_ptr<const UserService> user_service;
        std::weak_ptr<const ChatService> chat_service;
        std::weak_ptr<const MessageService> message_service;
        std::weak_ptr<const MembershipService> membership_service;
        std::weak_ptr<const PresenceService> presence_service;
        std::thread callback_thread;
        std::atomic<bool> thread_run_flag;
};

#endif // PN_CHAT_CALLBACK_SERVICE_HPP
