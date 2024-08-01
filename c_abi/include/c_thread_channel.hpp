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

#endif // PN_CHAT_C_THREAD_CHANNEL_H