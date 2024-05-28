#ifndef PN_CHAT_C_CHANNEL_H
#define PN_CHAT_C_CHANNEL_H

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

#endif // PN_CHAT_C_CHANNEL_H

