#include "rate_limiter.hpp"
#include "infra/sync.hpp"
#include "infra/timer.hpp"
#include "string.hpp"
#include <algorithm>
#include <chrono>

#define COLLECTOR_INTERVAL_MS 1000

ExponentialRateLimiter::ExponentialRateLimiter(float exponential_factor) :
    exponential_factor(exponential_factor),
    timers(std::make_unique<Mutex<std::list<Timer>>>()) {
        this->timer_collector = std::thread(
                [this] {
                while (!this->should_stop.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(COLLECTOR_INTERVAL_MS));

                    {
                        auto timers_lock = this->timers->lock();
                        if (timers_lock->empty()) {
                            continue;
                        }

                        timers_lock->remove_if([](Timer& timer) {
                            return !timer.is_active();
                        });
                    }
                }
            }
        );
}

ExponentialRateLimiter::~ExponentialRateLimiter() {
    this->should_stop.store(true);
    if (this->timer_collector.joinable()) {
        this->timer_collector.join();
    }
}

void ExponentialRateLimiter::run_within_limits(const Pubnub::String& id, int base_interval_ms, std::function<Pubnub::String()> task, std::function<void(Pubnub::String)> callback, std::function<void(std::exception&)> error_callback) {
    if (base_interval_ms == 0) {
        try {
            callback(task());
        } catch (std::exception& e) {
            error_callback(e);
        }

        return;
    }

    bool should_start_new_process = false;
    {
        auto limiter_guard = this->limiters.lock();
        auto limiter = limiter_guard->find(id);

        should_start_new_process = limiter == limiter_guard->end();
        if (should_start_new_process) {
            limiter = limiter_guard->insert({id, std::move(Mutex(std::move(RateLimiterRoot{{}, 0, base_interval_ms})))}).first;
            auto limiter_root = limiter->second.lock();

            limiter_root->queue.push_back({task, callback, error_callback, limiter_root->current_penalty});

        } else {
            auto limiter_root = limiter->second.lock();

            limiter_root->current_penalty += 1;

            limiter_root->queue.push_back({task, callback, error_callback, limiter_root->current_penalty});
        }
    }

    if (should_start_new_process) {
        this->process_queue(id);
    }
}

void ExponentialRateLimiter::process_queue(const Pubnub::String& id) {
    auto limiter_guard = this->limiters.lock();
    auto limiter = limiter_guard->find(id);

    if (limiter == limiter_guard->end()) {
        return;
    }

    auto limiter_root = limiter->second.lock();

    if (limiter_root->queue.empty()) {
        limiter_guard->erase(id);
 
        return;
    }
 
    auto element = limiter_root->queue.front();

    try {
        element.callback(element.task());
    } catch (std::exception& e) {
        element.error_callback(e);
    }

    limiter_root->queue.pop_front();

    this->timers->lock()->emplace_back(
            limiter_root->base_interval_ms * std::pow(this->exponential_factor, limiter_root->current_penalty),
            [this, id] {
                this->process_queue(id);
            }
        );
}

