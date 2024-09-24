#ifndef TIMER_H
#define TIMER_H

#include <atomic>
#include <chrono>
#include <future>
#include <functional>

class Timer
{
    public:
        Timer() = default;
        Timer(int duration_ms, std::function<void()> callback);
        ~Timer();
    
        Timer& operator=(Timer&& other);

        void stop();
        inline bool is_active() {return is_running;};
        void async_start(int duration_ms, std::function<void()> callback);
 
    private:
        std::atomic_bool is_running = false;
        int elapsed_time = 0;
        std::thread timer_thread;
};

#endif // TIMER_H
