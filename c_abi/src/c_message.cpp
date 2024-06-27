#include "c_functions/c_message.hpp"
#include "c_functions/c_errors.hpp"

void pn_message_delete(Pubnub::Message* message) {
    delete message;
}

Pubnub::Message* pn_message_edit_text(
        Pubnub::Message* message,
        const char* text) {
    try {
        return new Pubnub::Message(message->edit_text(text));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_message_text(
        Pubnub::Message* message,
        char* result) {
    try {
        auto text = message->text();
        strcpy(result, text.c_str());
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_message_delete_message(Pubnub::Message* message) {
    try {
        message->delete_message();
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCTribool pn_message_deleted(Pubnub::Message* message) {
    try {
        return message->deleted() ? PN_C_TRUE : PN_C_FALSE;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_UNKNOWN;
    }
}

void pn_message_get_timetoken(Pubnub::Message* message, char* result) {
    auto timetoken = message->get_timetoken();
    strcpy(result, timetoken.c_str());
}

int pn_message_get_data_type(Pubnub::Message* message) {
    return message->get_message_data().type;
}

void pn_message_get_data_text(Pubnub::Message* message, char* result) {
    auto data_text = message->get_message_data().text;
    strcpy(result, data_text.c_str());
}

void pn_message_get_data_channel_id(Pubnub::Message* message, char* result) {
    auto data_channel_id = message->get_message_data().channel_id;
    strcpy(result, data_channel_id.c_str());
}

void pn_message_get_data_user_id(Pubnub::Message* message, char* result) {
    auto data_user_id = message->get_message_data().user_id;
    strcpy(result, data_user_id.c_str());
}

void pn_message_get_data_meta(Pubnub::Message* message, char* result) {
    auto data_meta = message->get_message_data().meta;
    strcpy(result, data_meta.c_str());
}

void pn_message_get_data_message_actions(Pubnub::Message* message, char* result) {
    // TODO: implement
}
