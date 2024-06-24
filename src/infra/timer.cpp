#include "infra/timer.hpp"


void Timer::start(int duration_ms, std::function<void()> callback) 
{
    if(is_used) return;
    is_used = true;
    is_running = true;
    std::async(std::launch::async, [=](){
        async_start(duration_ms, callback);
    });

}

void Timer::stop()
{
    is_running = false;
}

void Timer::async_start(int duration_ms, std::function<void()> callback) 
{
    auto timer_task = std::async(std::launch::async, [=]()
    {
        if(!is_running) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        callback();
        is_running = false;
    });
    timer_task.wait();
}