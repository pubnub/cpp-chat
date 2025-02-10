#include "c_response.hpp"
#include "pubnub_chat/chat.hpp"
#include "application/chat_service.hpp"
#include "c_errors.hpp"
#include "infra/sync.hpp"
#include "string.hpp"

PnCResult pn_c_consume_response_buffer(Pubnub::Chat* chat, char* result) {
    try {
        auto lock = chat->get_chat_service()->response_buffer.lock();

        *lock += "]";
        strcpy(result, lock->c_str());

        lock->clear();
        *lock += "[";
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_c_append_to_response_buffer(const ChatService* chat, const char* data) {
    try {
        auto lock = chat->response_buffer.lock();
        if (lock->length() > 1) {
            *lock += ",";
        }

        *lock += data;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}
