#ifndef PN_CHAT_SUBSCRIPTION_HPP
#define PN_CHAT_SUBSCRIPTION_HPP


#include <memory>
extern "C" {
#include <pubnub_subscribe_event_listener_types.h>
#include <pubnub_subscribe_event_engine.h>
}

class Subscription {
    public:
        Subscription(pubnub_subscription_t* subscription);

        void close();
        void add_message_listener(pubnub_subscribe_message_callback_t callback);

    private:
        std::unique_ptr<pubnub_subscription_t, void (*)(pubnub_subscription_t*)> subscription;
};

#endif
