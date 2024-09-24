#include "infra/timer.hpp"

#define MAX_SLEEP 1000

Timer::~Timer()
{
    this->stop();
}

Timer::Timer(int duration_ms, std::function<void()> callback) 
{
    is_running.store(true);
    this->timer_thread = std::thread([this, duration_ms, callback](){
        async_start(duration_ms, callback);
    });
}

Timer& Timer::operator=(Timer&& other)
{
    if (this->is_running.load()) {
        this->stop();
    }

    this->is_running = other.is_running.load();
    this->elapsed_time = other.elapsed_time;
    this->timer_thread = std::move(other.timer_thread);

    return *this;
}

void Timer::stop()
{
    is_running.store(false);
    if (this->timer_thread.joinable()) {
        this->timer_thread.join();
    }
}

void Timer::async_start(int duration_ms, std::function<void()> callback) 
{
    while(this->is_running.load())
    {
        if (this->elapsed_time >= duration_ms) {
            this->elapsed_time = 0;
            is_running.store(false);
            callback();
            return;
        }

        const auto sleep_for = std::min(duration_ms - this->elapsed_time, MAX_SLEEP);
        this->elapsed_time += sleep_for;

        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_for));
    }
}
