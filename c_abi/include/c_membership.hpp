#ifndef PN_CHAT_C_MEMBERSHIP_H
#define PN_CHAT_C_MEMBERSHIP_H

#include "chat.hpp"
#include "membership.hpp"
#include "user.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_membership_delete(Pubnub::Membership* membership);

// @jakub.grzesiowski don't wrap it but as I understood it will be needed
// to be used with streams
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Membership* pn_membership_from_user(
        Pubnub::Chat* chat,
        Pubnub::User* user,
        const char* membership_json);

// @jakub.grzesiowski don't wrap it but as I understood it will be needed
// to be used with streams
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Membership* pn_membership_from_channel(
        Pubnub::Chat* chat,
        Pubnub::Channel* channel,
        const char* membership_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Membership* pn_membership_update_dirty(
        Pubnub::Membership* membership,
        const char* custom_object_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_membership_get_user_id(
        Pubnub::Membership* membership,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_membership_get_channel_id(
        Pubnub::Membership* membership,
        char* result);


#endif // PN_CHAT_C_MEMBERSHIP_H
