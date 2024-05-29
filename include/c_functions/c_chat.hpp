#ifndef PN_CHAT_C_CHAT_HPP
#define PN_CHAT_C_CHAT_HPP

#include "chat.hpp"
#include "export.hpp"
#include "extern.hpp"
#include "chat/channel.hpp"
#include "c_functions/c_channel.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Chat* pn_chat_new(
        const char* publish,
        const char* subscribe,
        const char* user_id); 

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_chat_delete(Pubnub::Chat* chat);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_create_public_conversation(
        Pubnub::Chat* chat,
        const char* channel_id,
        ChatChannelDataHelper channel_data);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_create_public_conversation_dirty(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        int channel_status,
        char* channel_type);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_update_channel(
        Pubnub::Chat* chat,
        std::string channel_id,
        ChatChannelDataHelper channel_data);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_update_channel_dirty(
        Pubnub::Chat* chat,
        std::string channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        int channel_status,
        char* channel_type);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_get_channel(
        Pubnub::Chat* chat,
        const char* channel_id);

// TODO: get channels...

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_chat_delete_channel(
        Pubnub::Chat* chat,
        const char* channel_id);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_chat_set_restrictions(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* channel_id,
        bool ban_user, 
        bool mute_user,
        const char* reason);


#endif // PN_CHAT_C_CHAT_HPP
