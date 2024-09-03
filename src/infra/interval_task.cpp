#include "interval_task.hpp"

IntervalTask::IntervalTask(std::function<void()> task, int interval_ms) {
    this->thread = std::thread([this, task, interval_ms] {
        while(this->running.load())
        {
            task();
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        }
    });
}
