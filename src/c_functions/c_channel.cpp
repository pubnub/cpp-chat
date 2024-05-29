#include "c_functions/c_channel.hpp"

void pn_channel_delete(Pubnub::Channel* channel) {
    delete channel;
}

Pubnub::ChatChannelData chat_channel_data_from_helper(ChatChannelDataHelper helper) {
    Pubnub::ChatChannelData data;
    data.channel_name = helper.channel_name;
    data.description = helper.description;
    data.custom_data_json = helper.custom_data_json;
    data.updated = helper.updated;
    data.status = helper.status;
    data.type = helper.type;

    return data;
}

Pubnub::Channel* pn_channel_new(Pubnub::Chat* chat, const char* channel_id, ChatChannelDataHelper channel_data) {
    auto converted_data = chat_channel_data_from_helper(channel_data);
    return new Pubnub::Channel(chat->create_public_conversation(channel_id, converted_data));
}

Pubnub::Channel* pn_channel_new_dirty(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        int channel_status,
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

void pn_channel_update(
        Pubnub::Channel* channel,
        ChatChannelDataHelper channel_data) {
    auto converted_data = chat_channel_data_from_helper(channel_data);
    channel->update(converted_data);
}

void pn_channel_update_dirty(
        Pubnub::Channel* channel,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        int channel_status,
        char* channel_type
        ) {
    Pubnub::ChatChannelData converted_data; 
    converted_data.channel_name = channel_name;
    converted_data.description = channel_description;
    converted_data.custom_data_json = channel_custom_data_json;
    converted_data.updated = channel_updated;
    converted_data.status = channel_status;
    converted_data.type = channel_type;

    return channel->update(converted_data);
}

void pn_channel_connect(Pubnub::Channel* channel) {
    channel->connect();
}

void pn_channel_disconnect(Pubnub::Channel* channel) {
    channel->disconnect();
}

void pn_channel_join(Pubnub::Channel* channel, const char* additional_params) {
    channel->join(additional_params);
}

void pn_channel_leave(Pubnub::Channel* channel) {
    channel->leave();
}

void pn_channel_delete_channel(Pubnub::Channel* channel) {
    channel->delete_channel();
}

void pn_channel_set_restrictions(
        Pubnub::Channel* channel,
        const char* user_id,
        bool ban_user,
        bool mute_user
        ) {
    channel->set_restrictions(user_id, ban_user, mute_user);
}

