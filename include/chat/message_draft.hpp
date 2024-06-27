#ifndef MESSAGE_DRAFT_H
#define MESSAGE_DRAFT_H

#include <vector>
#include <map>
#include <functional>
#include "enums.hpp"
#include "string.hpp"
#include "export.hpp"
#include "chat/channel.hpp"

namespace Pubnub
{
    class Chat;
    class Channel;
    class User;

    struct MessageDraftConfig
    {
        //"channel"  or "global"
        Pubnub::String user_suggestion_source = "channel"; 
        bool is_typing_indicator_triggered = true;
        int user_limit = 10;
        int channel_limit = 10;
    };

    struct TextLink
    {
        int start_index;
        int end_index;
        Pubnub::String link;
    };

    PN_CHAT_EXPORT class MessageDraft
    {
        public:

        PN_CHAT_EXPORT MessageDraft(Pubnub::Chat& in_chat, Pubnub::Channel in_channel, MessageDraftConfig in_draft_config = MessageDraftConfig());

        private:
        Pubnub::Chat& chat_obj;
        Pubnub::Channel channel;
        MessageDraftConfig draft_config;
        Pubnub::String value;
        Pubnub::String previous_value;
        std::map<int, Pubnub::User> mentioned_users;
        std::map<int, Pubnub::Channel> referenced_channels;
        std::vector<TextLink> text_links;
        //Pubub::Message quoted_message; this requires invalid objects handling as we can't gruarantee this message to be always valid
 
    };
}
#endif /* MESSAGE_DRAFT_H */
