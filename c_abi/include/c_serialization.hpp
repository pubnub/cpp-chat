#ifndef PN_CHAT_C_SERIALIZATION_H
#define PN_CHAT_C_SERIALIZATION_H

#include "channel.hpp"
#include "message.hpp"
#include "user.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"
#include "c_errors.hpp"

extern "C" {
	#include <pubnub_api_types.h>
	#include <pubnub_helper.h>
}

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Message* pn_deserialize_message(Pubnub::Chat* chat, pubnub_v2_message* message);

//Channel updates (not new channels)
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_deserialize_channel(Pubnub::Chat* chat, pubnub_v2_message* channel);

//User updates
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::User* pn_deserialize_user(Pubnub::Chat* chat, pubnub_v2_message* user);

//Events (json)
PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_deserialize_event(pubnub_v2_message* event, char* result);

//Presence - LIST OF USER IDs
PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_deserialize_presence(pubnub_v2_message* presence, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Message* pn_deserialize_message_update(Pubnub::Chat* chat, pubnub_v2_message* message_update);

//Membership updates
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Membership* pn_deserialize_membership(Pubnub::Chat* chat, pubnub_v2_message* membership);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_dispose_message(pubnub_v2_message* message);

#endif // PN_CHAT_C_SERIALIZATION_H
