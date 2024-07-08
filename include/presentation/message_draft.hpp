#ifndef PN_CHAT_MESSAGE_DRAFT_H
#define PN_CHAT_MESSAGE_DRAFT_H

#include "string.hpp"
#include <map>
#include <vector>

#include "presentation/message_draft_config.hpp"
#include "presentation/channel.hpp"
#include "presentation/user.hpp"

namespace Pubnub
{
    class Channel;
    class User;
    struct TextLink
    {
        int start_index;
        int end_index;
        Pubnub::String link;
    };

    PN_CHAT_EXPORT class MessageDraft
    {
        public:
            Pubnub::Channel channel;
            Pubnub::String value;
            MessageDraftConfig draft_config;

        private:
            PN_CHAT_EXPORT MessageDraft::MessageDraft(Pubnub::Channel channel, Pubnub::MessageDraftConfig draft_config = Pubnub::MessageDraftConfig());
        
            Pubnub::String previous_value;
            std::map<int, Pubnub::User> mentioned_users;
            std::map<int, Pubnub::Channel> referenced_channels;
            std::vector<TextLink> text_links;
            //Pubub::Message quoted_message; this requires invalid objects handling as we can't gruarantee this message to be always valid
    
            friend class ::MessageService;
    };
}
#endif /* PN_CHAT_MESSAGE_DRAFT_H */
