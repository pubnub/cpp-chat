#include "c_functions/c_membership.hpp"
#include "c_functions/c_errors.hpp"
#include "chat/membership.hpp"

void pn_membership_delete(Pubnub::Membership* membership) {
    delete membership;
}

Pubnub::Membership* pn_membership_from_user(
        Pubnub::Chat* chat,
        Pubnub::User* user,
        const char* membership_json) {
    try {
        return new Pubnub::Membership(*chat, *user, membership_json);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Membership* pn_membership_from_channel(
        Pubnub::Chat* chat,
        Pubnub::Channel* channel,
        const char* membership_json) {
    try {
        return new Pubnub::Membership(*chat, *channel, membership_json);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

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
    auto user_id = membership->get_user_id();
    strcpy(result, user_id.c_str());
}

void pn_membership_get_channel_id(
        Pubnub::Membership* membership,
        char* result) {
    auto channel_id = membership->get_channel_id();
    strcpy(result, channel_id.c_str());
}


