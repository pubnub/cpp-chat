#ifndef PN_CHAT_SUBSCRIPTION_HPP
#define PN_CHAT_SUBSCRIPTION_HPP

#include <functional>
#include <memory>

#include "enums.hpp"
#include "string.hpp"
#include <list>
#include <any>
#include <vector>

extern "C" {
#include <pubnub_subscribe_event_engine.h>
#include <pubnub_subscribe_event_engine_types.h>
#include <pubnub_subscribe_event_listener_types.h>
}

struct CCoreCallbackData;

class Subscribable {
    public:
        virtual void close() = 0;
};

class Subscription: public Subscribable {
    public:
        Subscription(pubnub_subscription_t* subscription);
        ~Subscription();

        virtual void close();
        void add_message_listener(CCoreCallbackData callback);
        void add_channel_update_listener(CCoreCallbackData callback);
        void add_user_update_listener(CCoreCallbackData callback);
        void add_event_listener(
            CCoreCallbackData callback,
            Pubnub::pubnub_chat_event_type event_type
        );
        void add_presence_listener(CCoreCallbackData callback);
        void add_membership_update_listener(CCoreCallbackData callback);
        void add_message_update_listener(CCoreCallbackData callback);
        void add_thread_message_update_listener(CCoreCallbackData callback);

    private:
        pubnub_subscription_t* subscription;
        std::vector<std::reference_wrapper<std::any>> contextes;

        void add_callback(
            CCoreCallbackData callback,
            pubnub_subscribe_listener_type type,
            const Pubnub::String& callback_kind
        );
};

class SubscriptionSet: public Subscribable {
    public:
        SubscriptionSet(pubnub_subscription_set_t* subscription_set);
        ~SubscriptionSet();

        virtual void close();
        void add_channel_update_listener(CCoreCallbackData callback);
        void add_membership_update_listener(CCoreCallbackData callback);
        void add_message_update_listener(CCoreCallbackData callback);
        void add_thread_message_update_listener(CCoreCallbackData callback);
        void add_user_update_listener(CCoreCallbackData callback);

    private:
        pubnub_subscription_set_t* subscription_set;
        std::vector<std::reference_wrapper<std::any>> contextes;

        void add_callback(
            CCoreCallbackData callback,
            pubnub_subscribe_listener_type type,
            const Pubnub::String& callback_kind
        );
};

#endif
