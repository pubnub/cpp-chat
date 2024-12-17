#include "c_thread_message.hpp"
#include "c_errors.hpp"
#include "message.hpp"
#include "thread_message.hpp"

void pn_thread_message_dispose(Pubnub::ThreadMessage* thread_message){
    delete thread_message;
}

Pubnub::ThreadMessage* pn_thread_message_consume_and_upgrade(
        Pubnub::Message* message,
        const char* parent_channel_id) {
    try {
        auto* thread_message = new Pubnub::ThreadMessage(*message, parent_channel_id);
        free(message);

        return thread_message;
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::ThreadMessage* pn_thread_message_edit_text(
    Pubnub::ThreadMessage* message,
    const char* text) {
    try {
        return new Pubnub::ThreadMessage(message->edit_text(text), message->parent_channel_id());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
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

PnCResult pn_thread_message_parent_channel_id(Pubnub::ThreadMessage* thread_message, char* result) {
    try {
        auto parent_id = thread_message->parent_channel_id();
        strcpy(result, parent_id.c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}
