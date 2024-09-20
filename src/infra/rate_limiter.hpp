#ifndef PN_CHAT_RATE_LIMITER_HPP
#define PN_CHAT_RATE_LIMITER_HPP

#include "infra/sync.hpp"
#include "string.hpp"
#include <deque>
#include <exception>
#include <functional>
#include <map>

struct RateLimiterElement {
    std::function<void*()> task;
    std::function<void(void*)> callback;
    std::function<void(std::exception&)> error_callback;
    int penalty;
};

struct RateLimiterRoot {
    std::deque<RateLimiterElement> queue = {};
    int current_penalty = 0;
    int base_interval_ms;
};

class ExponentialRateLimiter {
    public:
        ExponentialRateLimiter(float exponential_factor);

        void run_within_limits(const Pubnub::String& id, int base_interval_ms, std::function<void*()> task, std::function<void(void*)> callback, std::function<void(std::exception&)> error_callback);
    private:
        void process_queue(const Pubnub::String& id);

        float exponential_factor;
        std::map<Pubnub::String, Mutex<RateLimiterRoot>, Pubnub::StringComparer> limiters;
};

#endif // PN_CHAT_RATE_LIMITER_HPP
