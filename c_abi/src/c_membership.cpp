#include "c_membership.hpp"
#include "c_errors.hpp"
#include "membership.hpp"
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
        const char* custom_object_json) {
    try {
        return new Pubnub::Membership(membership->update(custom_object_json));
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

