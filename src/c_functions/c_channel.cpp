#include "c_functions/c_channel.hpp"
#include "callbacks.hpp"
#include "chat.hpp"
#include "chat/message.hpp"

void pn_channel_delete(Pubnub::Channel* channel) {
    delete channel;
}

Pubnub::Channel* pn_channel_new_dirty(
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

void pn_channel_update_dirty(
        Pubnub::Channel* channel,
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

    return channel->update(converted_data);
}

// TODO: dont copy code
char* jsonize_messages2(std::vector<Pubnub::Message> messages) {
    Pubnub::String result = "[";
    for (auto message : messages) {
        result += message.to_string();
    }   

    result += "]";
    
    char* c_result = new char[result.length() + 1];

    memcpy(c_result, result.c_str(), result.length() + 1);

    return c_result;
}


char* pn_channel_connect(Pubnub::Channel* channel, CallbackStringFunction callback) {
    auto messages = channel->connect_and_get_messages();

    return jsonize_messages2(messages);
}

void pn_channel_disconnect(Pubnub::Channel* channel) {
    channel->disconnect();
}

void pn_channel_join(Pubnub::Channel* channel, CallbackStringFunction callback) {
    channel->join(callback);
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
        bool ban,
        bool mute,
        const char* reason
        ) {
    Pubnub::PubnubRestrictionsData restrictions;
    restrictions.ban = ban;
    restrictions.mute = mute;
    restrictions.reason = reason;

    channel->set_restrictions(user_id, restrictions);
}

void pn_channel_send_text(
    Pubnub::Channel* channel,
    const char* message,
    Pubnub::pubnub_chat_message_type type,
    const char* metadata
    ) {
    channel->send_text(message, type, metadata);
}

