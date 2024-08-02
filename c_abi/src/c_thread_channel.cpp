#include "c_thread_channel.hpp"
#include "c_errors.hpp"
#include <iostream>
#include <sstream>

void pn_thread_channel_dispose(Pubnub::ThreadChannel* thread_channel) {
    delete thread_channel;
}

PnCResult pn_thread_channel_get_history(
    Pubnub::ThreadChannel* thread_channel,
    const char* start_timetoken,
    const char* end_timetoken,
    int count,
    char* thread_messages_pointers_json) {
        try {
            auto history = thread_channel->get_thread_history(start_timetoken, end_timetoken, count);
            if (history.size() == 0) {
                memcpy(thread_messages_pointers_json, "[]\0", 3);
                return PN_C_OK;
            }

            Pubnub::String string = "[";
            for (auto thread_message : history) {
                auto ptr = new Pubnub::ThreadMessage(thread_message);
                // TODO: utils void* to string
#ifdef _WIN32
                string += "0x";
#endif
                std::ostringstream oss;
                oss << static_cast<void*>(ptr);
                string += oss.str();
                string += ",";
            }

            string.erase(string.length() - 1);
            string += "]";

            strcpy(thread_messages_pointers_json, string.c_str());
        }
        catch (std::exception& e) {
            pn_c_set_error_message(e.what());

            return PN_C_ERROR;
        }

        return PN_C_OK;
}