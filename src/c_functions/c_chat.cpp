#include "c_functions/c_chat.hpp"

Pubnub::Chat* pn_chat_new(
        const char* publish,
        const char* subscribe,
        const char* user_id) {
    auto chat = new Pubnub::Chat();

    chat->init(publish, subscribe, user_id);

    return chat;
}

void pn_chat_delete(Pubnub::Chat* chat) {
    delete chat;
}

void pn_chat_send_message(
        Pubnub::Chat* chat,
        const char* message) {
    throw std::runtime_error("Not implemented");
}




