#ifndef PN_CHAT_TIMETOKEN_HPP
#define PN_CHAT_TIMETOKEN_HPP

#include "string.hpp"

namespace Timetoken {
    using Timetoken = Pubnub::String;

    Timetoken now();
    long long now_numeric();

    Timetoken increase_by(const Timetoken& timetoken, int value);
    Timetoken increase_by_one(const Timetoken& timetoken);

    long long to_long(const Timetoken& timetoken);
}

#endif // PN_CHAT_TIMETOKEN_HPP
