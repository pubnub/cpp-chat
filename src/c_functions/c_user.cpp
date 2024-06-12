#include "c_functions/c_user.hpp"
#include "c_functions/c_errors.hpp"
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

PnCResult pn_user_update_dirty(
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

    try {
        user->update(user_data);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_user_delete_user(Pubnub::User* user) {
    try {
        user->delete_user();
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_user_set_restrictions(
        Pubnub::User* user,
        const char* channel_id,
        bool ban_user,
        bool mute_user,
        const char* reason) {
    Pubnub::PubnubRestrictionsData restrictions;
    restrictions.ban = ban_user;
    restrictions.mute = mute_user;
    restrictions.reason = reason;

    try {
        user->set_restrictions(channel_id, restrictions);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_user_report(
        Pubnub::User* user,
        const char* reason) {
    try {
        user->report(reason);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_user_where_present(Pubnub::User* user, char* result_json) {
    std::vector<Pubnub::String> channels;

    try {
        channels = user->where_present();
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
    
    Pubnub::String result = "[";
    for (auto it = channels.begin(); it != channels.end(); ++it) {
        result += *it;
        if (it != channels.end() - 1) {
            result += ",";
        }
    }
    result += "]";

    strcpy(result_json, result.c_str());

    return PN_C_OK;
}

PnCTribool pn_user_is_present_on(Pubnub::User* user, const char* channel_id) {
    try {
        return user->is_present_on(channel_id) ? PN_C_TRUE : PN_C_FALSE;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_UNKNOWN;
    }
}

