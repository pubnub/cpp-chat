#include "pubnub_chat/callback_handle.hpp"
#include <memory>
#include "application/subscription.hpp"

Pubnub::CallbackHandle::CallbackHandle(std::shared_ptr<Subscribable> subscription) :
    subscription(subscription) {}

void Pubnub::CallbackHandle::close() {
    subscription->close();
}
