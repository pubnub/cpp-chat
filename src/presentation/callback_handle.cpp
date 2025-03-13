#include "pubnub_chat/callback_handle.hpp"
#include <memory>
#include "application/subscription.hpp"

Pubnub::CallbackHandle::CallbackHandle(std::shared_ptr<Subscribable> subscription) :
    subscription(subscription) {}

Pubnub::CallbackHandle::CallbackHandle(CallbackHandle& other) :
    subscription(other.subscription) {}

Pubnub::CallbackHandle::CallbackHandle(const CallbackHandle& other) :
    subscription(other.subscription) {}

void Pubnub::CallbackHandle::close() {
    subscription->close();
}
