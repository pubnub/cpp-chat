#include "c_functions/c_chat.hpp"
#include "c_functions/c_channel.hpp"
#include "chat/channel.hpp"

Pubnub::Chat* pn_chat_new(
        const char* publish,
        const char* subscribe,
        const char* user_id) {
    auto chat = new Pubnub::Chat(publish, subscribe, user_id);

    return chat;
}

void pn_chat_delete(Pubnub::Chat* chat) {
    delete chat;
}

Pubnub::Channel* pn_chat_create_public_conversation_dirty(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        char* channel_status,
        char* channel_type
        ) {
    Pubnub::ChatChannelData converted_data; 
    converted_data.channel_name = channel_name;
    converted_data.description = channel_description;
    converted_data.custom_data_json = channel_custom_data_json;
    converted_data.updated = channel_updated;
    converted_data.status = channel_status;
    converted_data.type = channel_type;

    return new Pubnub::Channel(chat->create_public_conversation(channel_id, converted_data));
}

Pubnub::Channel* pn_chat_update_channel_dirty(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        char* channel_status,
        char* channel_type) {
    Pubnub::ChatChannelData converted_data; 
    converted_data.channel_name = channel_name;
    converted_data.description = channel_description;
    converted_data.custom_data_json = channel_custom_data_json;
    converted_data.updated = channel_updated;
    converted_data.status = channel_status;
    converted_data.type = channel_type;

    return new Pubnub::Channel(chat->update_channel(channel_id, converted_data));
}

Pubnub::Channel* pn_chat_get_channel(
        Pubnub::Chat* chat,
        const char* channel_id) {
    return new Pubnub::Channel(chat->get_channel(channel_id));
}

void pn_chat_delete_channel(
        Pubnub::Chat* chat,
        const char* channel_id) {
    chat->delete_channel(channel_id);
}

void pn_chat_set_restrictions(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* channel_id,
        bool ban_user, 
        bool mute_user,
        const char* reason) {
    chat->set_restrictions(user_id, channel_id, ban_user, mute_user, reason);
}

