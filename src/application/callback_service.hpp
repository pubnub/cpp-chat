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
                std::weak_ptr<PresenceService> channel_service,
                ThreadSafePtr<PubNub> pubnub,
                milliseconds wait_interval = milliseconds(PUBNUB_WAIT_INTERVAL_MS)
        );
        ~CallbackService() {
            this->thread_run_flag.store(false);

            if (this->callback_thread.joinable()) {
                this->callback_thread.join();
            }
        };

    private:
        void resolve_callbacks();
        void broadcast_callbacks_from_message(pubnub_v2_message message);

        CallbacksRepository callbacks;
        ThreadSafePtr<PubNub> pubnub;
        std::weak_ptr<ChannelService> channel_service;
        std::weak_ptr<UserService> user_service;
        std::weak_ptr<ChatService> chat_service;
        std::weak_ptr<MessageService> message_service;
        std::weak_ptr<MembershipService> membership_service;
        std::weak_ptr<PresenceService> presence_service;
        std::thread callback_thread;
        std::atomic<bool> thread_run_flag;
};

#endif // PN_CHAT_CALLBACK_SERVICE_HPP
