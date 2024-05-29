#ifndef PN_CHAT_C_CHANNEL_H
#define PN_CHAT_C_CHANNEL_H

#include "chat.hpp"
#include "chat/channel.hpp"
#include "export.hpp"
#include "extern.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_delete(Pubnub::Channel* channel);

struct ChatChannelDataHelper {
    char* channel_name;
    char* description;
    char* custom_data_json;
    char* updated;
    int status;
    char* type;
};

Pubnub::ChatChannelData chat_channel_data_from_helper(ChatChannelDataHelper helper);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_channel_new(Pubnub::Chat* chat, const char* channel_id, ChatChannelDataHelper channel_data);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_channel_new_dirty(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        int channel_status,
        char* channel_type);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_update(
        Pubnub::Channel* channel,
        ChatChannelDataHelper channel_data);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_update_dirty(
        Pubnub::Channel* channel,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        int channel_status,
        char* channel_type);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_connect(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_disconnect(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_join(Pubnub::Channel* channel, const char* additional_params);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_leave(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_delete_channel(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_set_restrictions(
        Pubnub::Channel* channel,
        const char* user_id,
        bool ban_user,
        bool mute_user,
        const char* reason);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_send_text(
        Pubnub::Channel* channel,
        const char* message,
        Pubnub::pubnub_chat_message_type type,
        const char* metadata);

#endif // PN_CHAT_C_CHANNEL_H

