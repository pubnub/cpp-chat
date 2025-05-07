#include "c_membership.hpp"
#include "c_errors.hpp"
#include "c_response.hpp"
#include "membership.hpp"
#include "nlohmann/json.hpp"
#include <iostream>

void pn_membership_delete(Pubnub::Membership* membership) {
    delete membership;
}

//Pubnub::Membership* pn_membership_from_user(
//        Pubnub::Chat* chat,
//        Pubnub::User* user,
//        const char* membership_json) {
//    try {
//        return new Pubnub::Membership(*chat, *user, membership_json);
//    } catch (std::exception& e) {
//        pn_c_set_error_message(e.what());
//
//        return PN_C_ERROR_PTR;
//    }
//}
//
//Pubnub::Membership* pn_membership_from_channel(
//        Pubnub::Chat* chat,
//        Pubnub::Channel* channel,
//        const char* membership_json) {
//    try {
//        return new Pubnub::Membership(*chat, *channel, membership_json);
//    } catch (std::exception& e) {
//        pn_c_set_error_message(e.what());
//
//        return PN_C_ERROR_PTR;
//    }
//}

Pubnub::Membership* pn_membership_update_dirty(
        Pubnub::Membership* membership,
        const char* custom_data_json,
        const char* type,
        const char* status) {
    try {
        Pubnub::ChatMembershipData new_data;
        new_data.custom_data_json = custom_data_json;
        new_data.type = type;
        new_data.status = status;
        return new Pubnub::Membership(membership->update(new_data));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

void pn_membership_get_user_id(
        Pubnub::Membership* membership,
        char* result) {
    auto user_id = membership->user.user_id();
    strcpy(result, user_id.c_str());
}

void pn_membership_get_membership_data(Pubnub::Membership* membership, char* result) {
    auto membership_data = membership->membership_data();
    auto json = nlohmann::json {
        {"customDataJson", membership_data.custom_data_json.c_str()},
        {"type", membership_data.type.c_str()},
        {"status", membership_data.status.c_str()}
    };
    strcpy(result, json.dump().c_str());
}

void pn_membership_get_channel_id(
        Pubnub::Membership* membership,
        char* result) {
    auto channel_id = membership->channel.channel_id();
    strcpy(result, channel_id.c_str());
}

PnCResult pn_membership_last_read_message_timetoken(Pubnub::Membership* membership, char* result) {
    try {
        auto time_token = membership->last_read_message_timetoken();
        if (!time_token.empty()) {
            strcpy(result, time_token.c_str());
        }
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::Membership* pn_membership_set_last_read_message_timetoken(Pubnub::Membership* membership, const char* timetoken) {
    try {
        return new Pubnub::Membership(membership->set_last_read_message_timetoken(timetoken));
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}
Pubnub::Membership* pn_membership_set_last_read_message(Pubnub::Membership* membership, Pubnub::Message* message) {
    try {
        return new Pubnub::Membership(membership->set_last_read_message(*message));
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}
PnCResult pn_membership_get_unread_messages_count(Pubnub::Membership* membership) {
    try {
        return membership->get_unread_messages_count();
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::Membership* pn_membership_update_with_base(Pubnub::Membership* membership, Pubnub::Membership* base_membership) {
    try {
        return new Pubnub::Membership(membership->update_with_base(*base_membership));
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::CallbackHandle* pn_membership_stream_updates(Pubnub::Membership* membership) {
    try {
        auto chat = membership->shared_chat_service();
        return new Pubnub::CallbackHandle(membership->stream_updates([chat](const Pubnub::Membership& membership) {
                    pn_c_append_pointer_to_response_buffer(chat.get(), "membership_update", new Pubnub::Membership(membership));
        }));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}
