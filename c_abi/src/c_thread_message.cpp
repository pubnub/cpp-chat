#include "c_thread_message.hpp"
#include "c_errors.hpp"

void pn_thread_message_dispose(Pubnub::ThreadMessage* thread_message){
    delete thread_message;
}

PnCResult pn_thread_message_get_timetoken(Pubnub::ThreadMessage* thread_message, char* result) {
    try {
        strcpy(result, thread_message->timetoken());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}
