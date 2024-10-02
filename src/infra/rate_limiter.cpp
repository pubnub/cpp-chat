#include "rate_limiter.hpp"
#include "infra/sync.hpp"
#include "infra/timer.hpp"
#include "string.hpp"
#include <algorithm>
#include <chrono>
#include <memory>

#define THREADS_MAX_SPEEL_MS 1000

ExponentialRateLimiter::ExponentialRateLimiter(float exponential_factor) :
    exponential_factor(exponential_factor) {}

ExponentialRateLimiter::~ExponentialRateLimiter() {
    this->should_stop.store(true);
    if (nullptr != this->processor && this->processor->joinable()) {
        this->processor->join();
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

    auto limiter_guard = this->limiters.lock();
    auto limiter = limiter_guard->find(id);

    if (limiter == limiter_guard->end()) {
        limiter = limiter_guard->insert({id, RateLimiterRoot{{}, 0, base_interval_ms, 0, 0, false}}).first;

        if (limiter_guard->size() == 1) {
            if (nullptr != this->processor && this->processor->joinable()) {
                this->processor->join();
            }

            this->processor = this->processor_thread();
        }
    }

    auto& current = limiter->second;
    current.queue.push_back({task, callback, error_callback, current.current_penalty});
}

int ExponentialRateLimiter::process_queue(int slept_ms) {
    auto limiter_guard = this->limiters.lock();
    auto to_sleep = THREADS_MAX_SPEEL_MS;

    for (auto& [id, limiter] : *limiter_guard) {
        limiter.elapsed_ms += slept_ms;

        if (limiter.next_interval_ms > limiter.elapsed_ms) {
            to_sleep = std::min(to_sleep, limiter.next_interval_ms - limiter.elapsed_ms);
            continue;
        }

        this->process_limiter(id, limiter);

        limiter.current_penalty++;
        limiter.next_interval_ms = limiter.base_interval_ms * std::pow(this->exponential_factor, limiter.current_penalty);
        limiter.elapsed_ms = 0;

        to_sleep = std::min(to_sleep, limiter.next_interval_ms);
    }

    for (auto iter = limiter_guard->begin(); iter != limiter_guard->end();) {
        if (iter->second.finished) {
            iter = limiter_guard->erase(iter);
        } else {
            iter++;
        }
    }

    // TODO; find a better way
    if (limiter_guard->empty()) {
        to_sleep = -1;
    }

    return to_sleep;
}

void ExponentialRateLimiter::process_limiter(const Pubnub::String& id, RateLimiterRoot& limiter_root) {
    if (limiter_root.queue.empty()) {
        limiter_root.finished = true;
        return;
    }

    auto element = limiter_root.queue.front();

    try {
        element.callback(element.task());
    } catch (std::exception& e) {
        element.error_callback(e);
    }

    limiter_root.queue.pop_front();
}

std::unique_ptr<std::thread> ExponentialRateLimiter::processor_thread() {
    return std::make_unique<std::thread>([this] {
            auto slept = 0;
            auto to_sleep = 0;

            while (!this->should_stop.load()) {
                if (to_sleep == -1) {
                    break;
                }

                if (slept >= to_sleep) {
                    to_sleep = this->process_queue(slept);
                } else {
                    to_sleep -= slept;
                }

                slept = std::min(to_sleep, THREADS_MAX_SPEEL_MS);
                std::this_thread::sleep_for(
                        std::chrono::milliseconds(slept));
            }
    });
}

