#include "c_user.hpp"
#include "c_errors.hpp"
#include "c_response.hpp"
#include "channel.hpp"
#include "domain/quotes.hpp"
#include "page.hpp"
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
        return new Pubnub::User(user->update(user_data));
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

void pn_user_get_data_custom_data(Pubnub::User* user, char* result) {
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
        Pubnub::Channel* channel,
        char* result) {
    try {
        auto restrictions = user->get_channel_restrictions(*channel);
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
        const char* filter,
        const char* sort,
        const int limit,
        const char* next,
        const char* prev,
       char* result) {
    try {
        auto tuple = user->get_memberships(filter, sort, limit, {next, prev});

        std::vector<intptr_t> membership_pointers;
        for (auto membership : tuple.memberships) {
            auto ptr = new Pubnub::Membership(membership);
            membership_pointers.push_back(reinterpret_cast<intptr_t>(ptr));
        }   

        auto j = nlohmann::json{
            {"memberships", membership_pointers},
            {"total", tuple.total},
            {"status", tuple.status.c_str()},
            {"page", nlohmann::json {
                {"next", tuple.page.next.c_str()},
                {"prev", tuple.page.prev.c_str()}
            }}
        };

        strcpy(result, j.dump().c_str());
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::User* pn_user_update_with_base(Pubnub::User* user, Pubnub::User* base_user) {
    try {
        return new Pubnub::User(user->update_with_base(*base_user));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_user_get_channels_restrictions(Pubnub::User* user, const char* sort, int limit, const char* next, const char* prev, char* result) {
    try {
        auto wrapper = user->get_channels_restrictions(sort, limit, Pubnub::Page({next, prev}));
        std::vector<nlohmann::json> restrictions;
        for (auto restriction : wrapper.restrictions)
        {
            //TODO: put into util
            auto restriction_json = nlohmann::json{
                {"ban", restriction.ban},
                {"mute", restriction.mute},
                {"reason", restriction.reason.c_str()},
                {"channelId", restriction.channel_id.c_str()},
            };
            restrictions.push_back(restriction_json);
        }
        auto j = nlohmann::json{
                {"Restrictions", restrictions},
                {"Page", nlohmann::json{
                        {"Previous", wrapper.page.prev.c_str()},
                        {"Next", wrapper.page.next.c_str()},
                    }
                },
                {"Total", wrapper.total},
                {"Status", wrapper.status.c_str()},
        };
        strcpy(result, j.dump().c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCTribool pn_user_active(Pubnub::User* user) {
    try {
        return user->active() ? PN_C_TRUE : PN_C_FALSE;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_UNKNOWN;
    }
}


PnCResult pn_user_last_active_timestamp(Pubnub::User* user, char* result) {
    try {
        auto timestamp = user->last_active_timestamp();
        if (timestamp.has_value()) {
            strcpy(result, timestamp.value().c_str());
        } else {
            strcpy(result, "");
        }
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;

}

Pubnub::CallbackHandle* pn_user_stream_updates(Pubnub::User* user) {
    try {
        auto chat = user->shared_chat_service();

        return new Pubnub::CallbackHandle(user->stream_updates([chat](const Pubnub::User& user) {
                    pn_c_append_pointer_to_response_buffer(chat.get(), "user_update", new Pubnub::User(user));
        }));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}


