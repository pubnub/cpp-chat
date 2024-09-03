#include "timetoken.hpp"
#include <chrono>

Timetoken::Timetoken Timetoken::now() {
    return std::to_string(now_numeric());
}

long Timetoken::now_numeric() {
    // TODO: move time to infra for testing purposes
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

Timetoken::Timetoken Timetoken::increase_by(const Timetoken& timetoken, int value) {
    return std::to_string(to_long(timetoken) + value);
}

Timetoken::Timetoken Timetoken::increase_by_one(const Timetoken& timetoken) {
    return increase_by(timetoken, 1);
}

long Timetoken::to_long(const Timetoken& timetoken) {
    return std::stol(timetoken);
}
