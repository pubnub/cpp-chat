#ifndef PN_CHAT_C_THREAD_MESSAGE_H
#define PN_CHAT_C_THREAD_MESSAGE_H

#include "thread_channel.hpp"
#include "thread_message.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_thread_message_dispose(Pubnub::ThreadMessage* thread_message);

#endif // PN_CHAT_C_THREAD_MESSAGE_H