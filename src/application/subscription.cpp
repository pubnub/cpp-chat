#include "subscription.hpp"

#include <cstring>
#include <iostream>
#include <stdexcept>
extern "C" {
#include <pubnub_api_types.h>
#include <pubnub_helper.h>
#include <pubnub_subscribe_event_engine.h>
#include <pubnub_subscribe_event_engine_internal.h>
#include <pubnub_subscribe_event_engine_types.h>
#include <pubnub_subscribe_event_listener.h>
#include <pubnub_subscribe_event_listener_types.h>
}
#include "callback_service.hpp"
#include "enums.hpp"
#include "string.hpp"

static Pubnub::String error_message(
    const Pubnub::String& message,
    const Pubnub::String& kind,
    enum pubnub_res result
) {
    Pubnub::String error;
    auto result_str = pubnub_res_2_string(result);

    error.reserve(message.length() + kind.length() + std::strlen(result_str));

    error.replace(0, 0, message);
    error += " : ";
    error += result_str;
    error.replace_all("{}", kind.c_str());

    return error;
}

Subscription::Subscription(pubnub_subscription_t* subscription) : subscription(subscription) {}

Subscription::~Subscription() {
    pubnub_subscription_free(&this->subscription);
}

void Subscription::close() {
    auto result = pubnub_unsubscribe_with_subscription(&this->subscription);
    if (PNR_OK != result) {
        throw std::runtime_error(error_message("{} close failed", "Subscription", result).c_str());
    }
}

void Subscription::add_message_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_MESSAGE, "message");
}

void Subscription::add_channel_update_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_OBJECTS, "channel update");
}

void Subscription::add_user_update_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_OBJECTS, "user update");
}

void Subscription::add_event_listener(
    CCoreCallbackData callback,
    Pubnub::pubnub_chat_event_type event_type
) {
    switch (Pubnub::event_method_from_event_type(event_type)) {
        case Pubnub::EventMethod::Publish:
            this->add_callback(callback, PBSL_LISTENER_ON_MESSAGE, "event");
            break;
        case Pubnub::EventMethod::Signal:
            this->add_callback(callback, PBSL_LISTENER_ON_SIGNAL, "event");
            break;
        default:
            throw std::runtime_error("Unsupported event type");
    };
}

void Subscription::add_presence_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_MESSAGE, "presence");
}

void Subscription::add_membership_update_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_OBJECTS, "membership update");
}

void Subscription::add_message_update_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_MESSAGE_ACTION, "message update");
}

void Subscription::add_thread_message_update_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_MESSAGE_ACTION, "thread message update");
}

void Subscription::add_callback(
    CCoreCallbackData callback_data,
    pubnub_subscribe_listener_type type,
    const Pubnub::String& callback_kind
) {
    enum pubnub_res result = pubnub_subscribe_add_subscription_listener(
        subscription,
        type,
        callback_data.callback,
        &callback_data.context
    );

    this->contextes.push_back(callback_data.context);

    if (PNR_OK != result) {
        throw std::runtime_error(
            error_message("Couldn't add {} callback", callback_kind, result).c_str()
        );
    }

    result = pubnub_subscribe_with_subscription(subscription, NULL);

    if (PNR_OK != result) {
        throw std::runtime_error(
            error_message("Couldn't subscribe with {} subscription", callback_kind, result).c_str()
        );
    }
}

SubscriptionSet::SubscriptionSet(pubnub_subscription_set_t* subscription_set) :
    subscription_set(subscription_set) {}

SubscriptionSet::~SubscriptionSet() {
    pubnub_subscription_set_free(&this->subscription_set);
}

void SubscriptionSet::close() {
    auto result = pubnub_unsubscribe_with_subscription_set(&this->subscription_set);
    if (PNR_OK != result) {
        throw std::runtime_error(error_message("{} close failed", "SubscriptionSet", result).c_str()
        );
    }
}

void SubscriptionSet::add_channel_update_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_OBJECTS, "channels updates");
}

void SubscriptionSet::add_membership_update_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_OBJECTS, "memberships updates");
}

void SubscriptionSet::add_message_update_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_MESSAGE_ACTION, "message updates");
}

void SubscriptionSet::add_thread_message_update_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_MESSAGE_ACTION, "thread message updates");
}

void SubscriptionSet::add_user_update_listener(CCoreCallbackData callback) {
    this->add_callback(callback, PBSL_LISTENER_ON_OBJECTS, "users updates");
}

void SubscriptionSet::add_callback(
    CCoreCallbackData callback_data,
    pubnub_subscribe_listener_type type,
    const Pubnub::String& callback_kind
) {
    enum pubnub_res result = pubnub_subscribe_add_subscription_set_listener(
        this->subscription_set,
        type,
        callback_data.callback,
        &callback_data.context
    );

    this->contextes.push_back(callback_data.context);

    if (PNR_OK != result) {
        throw std::runtime_error(
            error_message("Couldn't add {} callback", callback_kind, result).c_str()
        );
    }

    result = pubnub_subscribe_with_subscription_set(this->subscription_set, NULL);

    if (PNR_OK != result) {
        throw std::runtime_error(
            error_message("Couldn't subscribe with {} subscription", callback_kind, result).c_str()
        );
    }
}
