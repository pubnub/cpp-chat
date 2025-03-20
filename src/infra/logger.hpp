#ifndef PN_CHAT_LOG_HPP
#define PN_CHAT_LOG_HPP

#include "enums.hpp"
#include "string.hpp"
#include <functional>
#include <sstream>

extern "C" {
#include <pubnub_log.h>
}


class Logger
{
    public:

    void log_message(Pubnub::pn_log_level log_level, Pubnub::String message);


    void register_logging_callback(std::function<void(Pubnub::pn_log_level, const char*)> callback);

    static void log_ccore_message(enum pubnub_log_level log_level, const char* message);

    private:

    static std::function<void(Pubnub::pn_log_level, const char*)> cpp_log_callback;

};

#endif // PN_CHAT_LOG_HPP
