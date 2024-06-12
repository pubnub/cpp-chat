#include "c_functions/c_membership.hpp"
#include "c_functions/c_errors.hpp"

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

PnCResult pn_membership_update_dirty(
        Pubnub::Membership* membership,
        const char* custom_object_json) {
    try {
        membership->update(custom_object_json);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}


