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
    std::deque<RateLimiterElement> queue;
    int current_penalty;
    int base_interval_ms;
    int next_interval_ms;
    int elapsed_ms;
    bool finished;
};

class ExponentialRateLimiter {
    public:
        ExponentialRateLimiter(float exponential_factor);
        ~ExponentialRateLimiter();

        void run_within_limits(const Pubnub::String& id, int base_interval_ms, std::function<Pubnub::String()> task, std::function<void(Pubnub::String)> callback, std::function<void(std::exception&)> error_callback);
    private:
        int process_queue(int slept_ms);
        void process_limiter(const Pubnub::String& id, RateLimiterRoot& limiter_root);
        std::thread processor_thread();
        std::thread garbage_collector_thread();

        float exponential_factor;
        Mutex<std::map<Pubnub::String, RateLimiterRoot, Pubnub::StringComparer>> limiters;
        std::thread processor;
        std::atomic_bool should_stop = false;
};

#endif // PN_CHAT_RATE_LIMITER_HPP
