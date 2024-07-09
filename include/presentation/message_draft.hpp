#ifndef PN_CHAT_MESSAGE_DRAFT_H
#define PN_CHAT_MESSAGE_DRAFT_H

#include "string.hpp"
#include <map>
#include <vector>

#include "option.hpp"
#include "presentation/text_link.hpp"
#include "presentation/message_draft_config.hpp"
#include "presentation/channel.hpp"
#include "presentation/user.hpp"
#include "presentation/message.hpp"

class MessageService;

namespace Pubnub
{
    class Channel;
    class User;

    PN_CHAT_EXPORT class MessageDraft
    {
        public:
            Pubnub::Channel channel;
            Pubnub::String value;
            MessageDraftConfig draft_config;

            PN_CHAT_EXPORT Pubnub::Message quoted_message();
            PN_CHAT_EXPORT void add_quote(Pubnub::Message message);
            PN_CHAT_EXPORT void remove_quote();

        private:
            PN_CHAT_EXPORT MessageDraft(Pubnub::Channel channel, Pubnub::MessageDraftConfig draft_config, std::shared_ptr<MessageService> message_service);

        
            Pubnub::String previous_value;
            std::map<int, Pubnub::User> mentioned_users;
            std::map<int, Pubnub::Channel> referenced_channels;
            std::vector<TextLink> text_links;
            Option<Pubnub::Message> quoted_message_internal;
    
            std::shared_ptr<MessageService> message_service;

            friend class ::MessageService;
    };
}
#endif /* PN_CHAT_MESSAGE_DRAFT_H */
