#include "chat.hpp"
#include "chat/user.hpp"
#include "export.hpp"
#include "extern.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::User* pn_user_create(Pubnub::Chat* chat, const char* user_id, Pubnub::ChatUserData user_data);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::User* pn_user_create_dirty(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* user_data_json,
        Pubnub::String user_name,
        Pubnub::String external_id,
        Pubnub::String profile_url,
        Pubnub::String email,
        Pubnub::String custom_data_json,
        int status,
        Pubnub::String type
        );
