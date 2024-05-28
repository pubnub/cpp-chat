#include "c_functions/c_chat.hpp"
#include "chat/channel.hpp"

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

Pubnub::Channel* pn_chat_create_public_conversation(
        Pubnub::Chat* chat,
        const char* channel_id,
        Pubnub::ChatChannelDataChar channel_data) {
    return chat->create_public_conversation((const char*)channel_id, channel_data);
}

Pubnub::Channel* pn_chat_update_channel(
        Pubnub::Chat* chat,
        const char* channel_id,
        Pubnub::ChatChannelDataChar channel_data) {
    return chat->update_channel(channel_id, channel_data);
}

Pubnub::Channel* pn_chat_get_channel(
        Pubnub::Chat* chat,
        const char* channel_id) {
    return chat->get_channel(channel_id);
}


