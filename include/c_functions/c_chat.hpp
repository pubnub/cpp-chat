#include "chat.hpp"
#include "export.hpp"

PN_CHAT_EXPORT Pubnub::Chat* pn_chat_new(
        const char* publish,
        const char* subscribe,
        const char* user_id); 

PN_CHAT_EXPORT void pn_chat_delete(Pubnub::Chat* chat);

PN_CHAT_EXPORT void pn_chat_send_message(
        Pubnub::Chat* chat,
        const char* message); 
