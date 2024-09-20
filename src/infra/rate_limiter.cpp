#include "rate_limiter.hpp"
#include "infra/sync.hpp"
#include "infra/timer.hpp"
#include "string.hpp"

ExponentialRateLimiter::ExponentialRateLimiter(float exponential_factor)
    : exponential_factor(exponential_factor) {}

void ExponentialRateLimiter::run_within_limits(const Pubnub::String& id, int base_interval_ms, std::function<Pubnub::String()> task, std::function<void(Pubnub::String)> callback, std::function<void(std::exception&)> error_callback) {
    if (base_interval_ms == 0) {
        try {
            auto result = task();
            callback(result);
        } catch (std::exception& e) {
            error_callback(e);
        }

        return;
    }

    auto limiter = this->limiters.find(id);

    if (limiter == this->limiters.end()) {
        limiter = this->limiters.insert({id, std::move(Mutex(std::move(RateLimiterRoot{.base_interval_ms = base_interval_ms})))}).first;
        {
            auto limiter_root = limiter->second.lock();

            limiter_root->queue.push_back({task, callback, error_callback, limiter_root->current_penalty});
        }

        this->process_queue(id);
    } else {
        auto limiter_root = limiter->second.lock();

        limiter_root->current_penalty += 1;

        limiter_root->queue.push_back({task, callback, error_callback, limiter_root->current_penalty});
    }
}

void ExponentialRateLimiter::process_queue(const Pubnub::String& id) {
    auto limiter = this->limiters.find(id);

    if (limiter == this->limiters.end()) {
        return;
    }

    auto limiter_root = limiter->second.lock();

    if (limiter_root->queue.empty()) {
        this->limiters.erase(id);

        return;
    }

    auto element = limiter_root->queue.front();

    try {
        auto result = element.task();
        element.callback(result);
    } catch (std::exception& e) {
        element.error_callback(e);
    }

    limiter_root->queue.pop_front();

    Timer().start(
            limiter_root->base_interval_ms * std::pow(this->exponential_factor, limiter_root->current_penalty),
            [this, id] {
                this->process_queue(id);
            }
    );
}

