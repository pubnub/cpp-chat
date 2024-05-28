#include "c_functions/c_chat.hpp"
#include "c_functions/c_channel.hpp"
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
        ChatChannelDataHelper channel_data) {
    auto converted_data = chat_channel_data_from_helper(channel_data);
    return new Pubnub::Channel(chat->create_public_conversation(channel_id, converted_data));
}

Pubnub::Channel* pn_chat_update_channel(
        Pubnub::Chat* chat,
        const char* channel_id,
        ChatChannelDataHelper channel_data) {
    auto converted_data = chat_channel_data_from_helper(channel_data);
    return new Pubnub::Channel(chat->update_channel(channel_id, converted_data));
}

Pubnub::Channel* pn_chat_get_channel(
        Pubnub::Chat* chat,
        const char* channel_id) {
    return new Pubnub::Channel(chat->get_channel(channel_id));
}


