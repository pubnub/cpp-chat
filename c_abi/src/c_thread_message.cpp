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

Pubnub::Channel* pn_thread_message_pin_to_parent_channel(Pubnub::ThreadMessage* thread_message)
{
    try {
        return new Pubnub::Channel(thread_message->pin_to_parent_channel());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Channel* pn_thread_message_unpin_from_parent_channel(Pubnub::ThreadMessage* thread_message)
{
    try {
        return new Pubnub::Channel(thread_message->unpin_from_parent_channel());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}
