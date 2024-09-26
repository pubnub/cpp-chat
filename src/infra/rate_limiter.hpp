#ifndef PN_CHAT_RATE_LIMITER_HPP
#define PN_CHAT_RATE_LIMITER_HPP

#include "infra/sync.hpp"
#include "infra/timer.hpp"
#include "string.hpp"
#include <atomic>
#include <deque>
#include <exception>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <vector>

struct RateLimiterElement {
    std::function<Pubnub::String()> task;
    std::function<void(Pubnub::String)> callback;
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
        ~ExponentialRateLimiter();

        void run_within_limits(const Pubnub::String& id, int base_interval_ms, std::function<Pubnub::String()> task, std::function<void(Pubnub::String)> callback, std::function<void(std::exception&)> error_callback);
    private:
        void process_queue(const Pubnub::String& id);

        float exponential_factor;
        std::map<Pubnub::String, Mutex<RateLimiterRoot>, Pubnub::StringComparer> limiters;
        std::unique_ptr<Mutex<std::list<Timer>>> timers;
        std::thread timer_collector;
        std::atomic_bool should_stop = false;
};

#endif // PN_CHAT_RATE_LIMITER_HPP
