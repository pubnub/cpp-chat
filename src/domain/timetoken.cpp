#include "timetoken.hpp"

Timetoken::Timetoken Timetoken::increase_by(const Timetoken& timetoken, int value) {
    return std::to_string(std::stoull(timetoken) + value);
}

Timetoken::Timetoken Timetoken::increase_by_one(const Timetoken& timetoken) {
    return increase_by(timetoken, 1);
}
