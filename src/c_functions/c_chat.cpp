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
        bool ban, 
        bool mute,
        const char* reason) {
    Pubnub::PubnubRestrictionsData restrictions;
    restrictions.ban = ban;
    restrictions.mute= mute;
    restrictions.reason = reason;

    chat->set_restrictions(user_id, channel_id, restrictions);
}

Pubnub::User* pn_chat_create_user_dirty(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* user_name,
        const char* external_id,
        const char* profile_url,
        const char* email,
        const char* custom_data_json,
        const char* status,
        const char* type) {
    Pubnub::ChatUserData converted_data; 
    converted_data.user_name = user_name;
    converted_data.external_id = external_id;
    converted_data.profile_url = profile_url;
    converted_data.email = email;
    converted_data.custom_data_json = custom_data_json;
    converted_data.status = status;
    converted_data.type = type;
    
    return new Pubnub::User(chat->create_user(user_id, converted_data));
}

Pubnub::User* pn_chat_get_user(
        Pubnub::Chat* chat,
        const char* user_id) {
    return new Pubnub::User(chat->get_user(user_id));
}

//std::vector<Pubnub::User>* pn_chat_get_users(
//        Pubnub::Chat* chat,
//        const char* include,
//        const int limit,
//        const char* start,
//        const char* end) {
//    return ->get_users(include, limit, start, end);
//}

Pubnub::User* pn_chat_update_user_dirty(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* user_name,
        const char* external_id,
        const char* profile_url,
        const char* email,
        const char* custom_data_json,
        const char* status,
        const char* type) {
    Pubnub::ChatUserData converted_data; 
    converted_data.user_name = user_name;
    converted_data.external_id = external_id;
    converted_data.profile_url = profile_url;
    converted_data.email = email;
    converted_data.custom_data_json = custom_data_json;
    converted_data.status = status;
    converted_data.type = type;
    
    return new Pubnub::User(chat->update_user(user_id, converted_data));
}

void pn_chat_delete_user(
        Pubnub::Chat* chat,
        const char* user_id) {
    chat->delete_user(user_id);
}

const char* jsonize_messages(std::vector<Pubnub::Message> messages) {
    if (messages.empty()) {
        return Pubnub::String("[]");
    }

    Pubnub::String result = "[";
    for (auto message : messages) {
        result += message.to_string();
        result += ",";
    }   

    result.erase(result.length() - 1);
    result += "]";
    
    char* c_result = new char[result.length() + 1];

    memcpy(c_result, result.c_str(), result.length() + 1);

    return c_result;
}

void pn_chat_get_messages(Pubnub::Chat *chat, const char *channel_id, char* messages_json) {
    auto messages = chat->get_pubnub_context().fetch_messages();
    auto jsonised = jsonize_messages(messages);
    strcpy(messages_json, jsonised);
    delete[] jsonised;
}

void pn_clear_string(char* str) {
    delete[] str;
}

