#include "c_functions/c_user.hpp"
#include "chat/user.hpp"

Pubnub::User* pn_user_create_dirty(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* user_data_json,
        const char* user_name,
        const char* external_id,
        const char* profile_url,
        const char* email,
        const char* custom_data_json,
        const char* status,
        const char* type) {
    Pubnub::Chat& chat_obj = *chat;
    Pubnub::ChatUserData user_data;

    user_data.user_name = user_name;
    user_data.external_id = external_id;
    user_data.profile_url = profile_url;
    user_data.email = email;
    user_data.custom_data_json = custom_data_json;
    user_data.status = status;
    user_data.type = type;

    return new Pubnub::User(chat_obj, user_id, user_data);
}

void pn_user_destroy(Pubnub::User* user) {
    delete user;
}

void pn_user_update_dirty(
        Pubnub::User* user,
        const char* user_data_json,
        const char* user_name,
        const char* external_id,
        const char* profile_url,
        const char* email,
        const char* custom_data_json,
        const char* status,
        const char* type) {
    Pubnub::ChatUserData user_data;

    user_data.user_name = user_name;
    user_data.external_id = external_id;
    user_data.profile_url = profile_url;
    user_data.email = email;
    user_data.custom_data_json = custom_data_json;
    user_data.status = status;
    user_data.type = type;

    user->update(user_data);
}

void pn_user_delete_user(Pubnub::User* user) {
    user->delete_user();
}

void pn_user_set_restrictions(
        Pubnub::User* user,
        const char* channel_id,
        bool ban_user,
        bool mute_user,
        const char* reason) {
    user->set_restrictions(channel_id, ban_user, mute_user, reason);
}

void pn_user_report(
        Pubnub::User* user,
        const char* reason) {
    user->report(reason);
}

const char** pn_user_where_present(Pubnub::User* user) {
    std::vector<Pubnub::String> channels = user->where_present();
    const char** channels_arr = new const char*[channels.size() + 1];
    for (int i = 0; i < channels.size(); i++) {
        channels_arr[i] = channels[i].c_str();
    }
    channels_arr[channels.size()] = nullptr;
    return channels_arr;
}

bool pn_user_is_present_on(Pubnub::User* user, const char* channel_id) {
    return user->is_present_on(channel_id);
}


