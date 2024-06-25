#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <future>
#include <functional>

class Timer
{
    public:
    Timer() = default;

    void start(int duration_ms, std::function<void()> callback);

    void stop();

    inline bool is_active() {return is_running;};

    private:
    void async_start(int duration_ms, std::function<void()> callback);
    bool is_running = false;
    bool is_used = false;
};

#endif // TIMER_H
