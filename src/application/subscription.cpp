#include "subscription.hpp"
#include <iostream>
extern "C" {
#include <pubnub_api_types.h>
#include <pubnub_helper.h>
#include <pubnub_subscribe_event_engine.h>
#include <pubnub_subscribe_event_listener.h>
#include <pubnub_subscribe_event_listener_types.h>
}
#include "string.hpp"

static void free_subscription(pubnub_subscription_t* subscription) {
    pubnub_subscription_free(&subscription);
    subscription = nullptr;
}

Subscription::Subscription(pubnub_subscription_t* subscription) :
    subscription(subscription, free_subscription) {}

void Subscription::close() {
    pubnub_subscription_t* raw_ptr = this->subscription.get();
    pubnub_unsubscribe_with_subscription(&raw_ptr);
}

void Subscription::add_message_listener(pubnub_subscribe_message_callback_t callback) {
    enum pubnub_res result = pubnub_subscribe_add_subscription_listener(
        subscription.get(),
        PBSL_LISTENER_ON_MESSAGE,
        callback
    );

    if (PNR_OK != result) {
        throw std::runtime_error((Pubnub::String("Couldn't add message callback: ")
                                  + pubnub_res_2_string(result))
                                     .to_std_string());
    }

    result = pubnub_subscribe_with_subscription(
        subscription.get(),
        NULL);

    if (PNR_OK != result) {
        throw std::runtime_error((Pubnub::String("Couldn't subscribe with message subscription: ")
                                  + pubnub_res_2_string(result))
                                     .to_std_string());
    }


}
