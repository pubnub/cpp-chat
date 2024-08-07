#ifndef PN_CHAT_C_THREAD_MESSAGE_H
#define PN_CHAT_C_THREAD_MESSAGE_H

#include "thread_channel.hpp"
#include "thread_message.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"
#include "c_errors.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_thread_message_dispose(Pubnub::ThreadMessage* thread_message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_get_timetoken(Pubnub::ThreadMessage* thread_message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_thread_message_unpin_from_parent_channel(Pubnub::ThreadMessage* thread_message);
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_thread_message_pin_to_parent_channel(Pubnub::ThreadMessage* thread_message);


#endif // PN_CHAT_C_THREAD_MESSAGE_H