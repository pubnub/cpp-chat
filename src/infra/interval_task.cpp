#include "interval_task.hpp"

#define MAX_SLEEP 1000

IntervalTask::IntervalTask(std::function<void()> task, int interval_ms) {
    this->thread = std::thread([this, task, interval_ms] {
        task();

        while(this->running.load())
        {
            if (this->elapsed_time >= interval_ms) {
                this->elapsed_time = 0;
                task();
            }
            
            const auto sleep_for = std::min(interval_ms - this->elapsed_time, MAX_SLEEP);
            this->elapsed_time += sleep_for;

            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_for));
        }
    });
}
