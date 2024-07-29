#include "c_user.hpp"
#include "c_errors.hpp"
#include "restrictions.hpp"
#include "user.hpp"
#include "nlohmann/json.hpp"
#include "membership.hpp"
#include <sstream>

//TODO: is that even needed?
//Pubnub::User* pn_user_create_dirty(
//        Pubnub::Chat* chat,
//        const char* user_id,
//        const char* user_data_json,
//        const char* user_name,
//        const char* external_id,
//        const char* profile_url,
//        const char* email,
//        const char* custom_data_json,
//        const char* status,
//        const char* type) {
//    Pubnub::Chat& chat_obj = *chat;
//    Pubnub::ChatUserData user_data;
//
//    user_data.user_name = user_name;
//    user_data.external_id = external_id;
//    user_data.profile_url = profile_url;
//    user_data.email = email;
//    user_data.custom_data_json = custom_data_json;
//    user_data.status = status;
//    user_data.type = type;
//
//    return new Pubnub::User(chat_obj, user_id, user_data);
//}

void pn_user_destroy(Pubnub::User* user) {
    delete user;
}

Pubnub::User* pn_user_update_dirty(
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
        new Pubnub::User(user->update(user_data));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
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
    Pubnub::Restriction restrictions;
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
        channels = user->where_present().into_std_vector();
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    if (channels.empty()) {
        memcpy(result_json, "[]\0", 3);

        return PN_C_OK;
    }
    
    Pubnub::String result = "[";
    for (auto channel : channels) {
        result += "\"";
        result += channel;
        result += "\",";
    }

    result.erase(result.length() - 1);
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

void pn_user_get_user_id(Pubnub::User* user, char* result) {
    auto user_id = user->user_id();
    strcpy(result, user_id.c_str());
}

void pn_user_get_data_user_name(Pubnub::User* user, char* result) {
    auto user_data = user->user_data().user_name;
    strcpy(result, user_data.c_str());
}

void pn_user_get_data_external_id(Pubnub::User* user, char* result) {
    auto user_data = user->user_data().external_id;
    strcpy(result, user_data.c_str());
}

void pn_user_get_data_profile_url(Pubnub::User* user, char* result) {
    auto user_data = user->user_data().profile_url;
    strcpy(result, user_data.c_str());
}

void pn_user_get_data_email(Pubnub::User* user, char* result) {
    auto user_data = user->user_data().email;
    strcpy(result, user_data.c_str());
}

void pn_user_get_data_custom_data_json(Pubnub::User* user, char* result) {
    auto user_data = user->user_data().custom_data_json;
    strcpy(result, user_data.c_str());
}

void pn_user_get_data_status(Pubnub::User* user, char* result) {
    auto user_data = user->user_data().status;
    strcpy(result, user_data.c_str());
}

void pn_user_get_data_type(Pubnub::User* user, char* result) {
    auto user_data = user->user_data().type;
    strcpy(result, user_data.c_str());
}

// TODO: utils
static void restrictions_to_json(nlohmann::json& j, const Pubnub::Restriction& data) {
    j = nlohmann::json{
        {"ban", data.ban},
        {"mute", data.mute},
        {"reason", data.reason.c_str()}
    };
}

PnCResult pn_user_get_channel_restrictions(
        Pubnub::User* user,
        const char* user_id,
        const char* channel_id,
        int limit,
        const char* start,
        const char* end,
        char* result) {
    try {
        auto restrictions = user->get_channel_restrictions(user_id, channel_id, limit, start, end);
        nlohmann::json json;
        restrictions_to_json(json, restrictions);

        strcpy(result, json.dump().c_str());
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_user_get_memberships(
        Pubnub::User* user,
        int limit,
        const char* start,
        const char* end,
        char* result) {
    try {
        auto memberships = user->get_memberships(limit, start, end);

        Pubnub::String string = "[";
        for (auto membership : memberships) {
            auto ptr = new Pubnub::Membership(membership);
            // TODO: utils void* to string
#ifdef _WIN32
            string += "0x";
#endif
            std::ostringstream oss;
            oss << static_cast<void*>(ptr);
            string += oss.str();
            string += ",";
        }   

        string.erase(string.length() - 1);
        string += "]";

        strcpy(result, string.c_str());
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}
