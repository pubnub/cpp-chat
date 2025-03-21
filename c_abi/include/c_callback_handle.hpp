#ifndef PN_CHAT_C_CALLBACK_HANDLE_HPP
#define PN_CHAT_C_CALLBACK_HANDLE_HPP

#include "c_errors.hpp"
#include "callback_handle.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_callback_handle_dispose(Pubnub::CallbackHandle* handle);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_callback_handle_close(Pubnub::CallbackHandle* handle);

#endif // PN_CHAT_C_CALLBACK_HANDLE_HPP
