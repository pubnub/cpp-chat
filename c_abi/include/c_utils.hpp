#ifndef PN_CHAT_C_UTILS_HPP
#define PN_CHAT_C_UTILS_HPP

#include <sstream>
#include "string.hpp"

template <typename T>
Pubnub::String to_ptr_string(T* ptr) {
    Pubnub::String result;
    result.reserve(18); //ptr str length

#ifdef _WIN32 
    result += "0x";
#endif

    std::ostringstream ss;
    ss << static_cast<void*>(ptr);

    result += ss.str();

    return result;
}

#endif
