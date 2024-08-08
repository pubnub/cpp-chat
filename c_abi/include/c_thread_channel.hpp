#ifndef PN_CHAT_C_THREAD_CHANNEL_H
#define PN_CHAT_C_THREAD_CHANNEL_H

#include "thread_channel.hpp"
#include "thread_message.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"
#include "c_errors.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_thread_channel_dispose(
	Pubnub::ThreadChannel* thread_channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_channel_get_history(
	Pubnub::ThreadChannel* thread_channel, 
	const char* start_timetoken, 
	const char* end_timetoken, 
	int count, 
	char* thread_messages_pointers_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_thread_channel_pin_message_to_parent_channel(Pubnub::ThreadChannel* thread_channel, Pubnub::ThreadMessage* message);
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_thread_channel_unpin_message_from_parent_channel(Pubnub::ThreadChannel* thread_channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_channel_send_text(Pubnub::ThreadChannel* thread_channel, const char* text);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_channel_get_parent_channel_id(Pubnub::ThreadChannel* thread_channel, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_channel_emit_user_mention(Pubnub::ThreadChannel* thread_channel, const char* user_id, const char* timetoken, const char* text);

#endif // PN_CHAT_C_THREAD_CHANNEL_H