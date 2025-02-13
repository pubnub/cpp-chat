#ifndef PN_CHAT_SUBSCRIPTION_HPP
#define PN_CHAT_SUBSCRIPTION_HPP

#include <memory>

#include "enums.hpp"
#include "string.hpp"

extern "C" {
#include <pubnub_subscribe_event_engine.h>
#include <pubnub_subscribe_event_engine_types.h>
#include <pubnub_subscribe_event_listener_types.h>
}

class Subscribable {
    public:
        virtual void close() = 0;
};

class Subscription: public Subscribable {
    public:
        Subscription(pubnub_subscription_t* subscription);
        ~Subscription();

        virtual void close();
        void add_message_listener(pubnub_subscribe_message_callback_t callback);
        void add_channel_update_listener(pubnub_subscribe_message_callback_t callback);
        void add_user_update_listener(pubnub_subscribe_message_callback_t callback);
        void add_event_listener(
            pubnub_subscribe_message_callback_t callback,
            Pubnub::pubnub_chat_event_type event_type
        );
        void add_presence_listener(pubnub_subscribe_message_callback_t callback);
        void add_membership_update_listener(pubnub_subscribe_message_callback_t callback);
        void add_message_update_listener(pubnub_subscribe_message_callback_t callback);
        void add_thread_message_update_listener(pubnub_subscribe_message_callback_t callback);

    private:
        pubnub_subscription_t* subscription;

        void add_callback(
            pubnub_subscribe_message_callback_t callback,
            pubnub_subscribe_listener_type type,
            const Pubnub::String& callback_kind
        );
};

class SubscriptionSet: public Subscribable {
    public:
        SubscriptionSet(pubnub_subscription_set_t* subscription_set);
        ~SubscriptionSet();

        virtual void close();
        void add_channel_update_listener(pubnub_subscribe_message_callback_t callback);
        void add_membership_update_listener(pubnub_subscribe_message_callback_t callback);
        void add_message_update_listener(pubnub_subscribe_message_callback_t callback);
        void add_thread_message_update_listener(pubnub_subscribe_message_callback_t callback);
        void add_user_update_listener(pubnub_subscribe_message_callback_t callback);

    private:
        pubnub_subscription_set_t*
            subscription_set;

        void add_callback(
            pubnub_subscribe_message_callback_t callback,
            pubnub_subscribe_listener_type type,
            const Pubnub::String& callback_kind
        );
};

#endif
