#include "infra/timer.hpp"

Timer::~Timer()
{
    this->stop();
}

void Timer::start(int duration_ms, std::function<void()> callback) 
{
    if(is_used) return;
    is_used = true;
    is_running = true;
    std::thread([=](){
        async_start(duration_ms, callback);
    }).detach();

}

void Timer::stop()
{
    is_running = false;
}

void Timer::async_start(int duration_ms, std::function<void()> callback) 
{
    if(!is_running) return;
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    callback();
    is_running = false;
}
