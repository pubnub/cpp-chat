#ifndef PN_CHAT_INTERVAL_TASK_HPP
#define PN_CHAT_INTERVAL_TASK_HPP

#include <atomic>
#include <functional>
#include <thread>

class IntervalTask
{
    public: 
        IntervalTask(std::function<void()> task, int interval_ms);
        ~IntervalTask() {
            running.store(false);
            thread.join();
        }

    private:
        std::atomic_bool running = true;
        std::thread thread;
};

#endif // PN_CHAT_INTERVAL_TASK_HPP
