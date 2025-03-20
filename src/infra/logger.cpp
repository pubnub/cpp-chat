#include "logger.hpp"
#include <iostream>

std::function<void(Pubnub::pn_log_level, const char*)> Logger::cpp_log_callback = nullptr;

void Logger::log_message(Pubnub::pn_log_level log_level, Pubnub::String message)
{
    if(cpp_log_callback)
    {
        cpp_log_callback(log_level, message);
    }
    else
    {
        std::cout << message << std::endl;
    }
}

void Logger::register_logging_callback(std::function<void(Pubnub::pn_log_level, const char*)> callback)
{
    cpp_log_callback = callback;
}

void Logger::log_ccore_message(enum pubnub_log_level log_level, const char* message)
{
    if(cpp_log_callback)
    {
        cpp_log_callback((Pubnub::pn_log_level)log_level, message);
    }
}