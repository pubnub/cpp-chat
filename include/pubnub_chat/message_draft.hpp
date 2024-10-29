#ifndef PN_CHAT_MESSAGE_DRAFT_H
#define PN_CHAT_MESSAGE_DRAFT_H

#include "string.hpp"
#include <map>
#include <vector>

#include "option.hpp"
#include "message_elements.hpp"
#include "channel.hpp"
#include "user.hpp"
#include "message.hpp"

class MessageService;
class DraftService;
class MessageDraftDAO;

namespace Pubnub
{
    class Channel;
    class User;

    PN_CHAT_EXPORT class MentionTarget {
        public:
            static MentionTarget user(const Pubnub::String& user_id);
            static MentionTarget channel(const Pubnub::String& channel);
            static MentionTarget url(const Pubnub::String& url);
        
        private:
            MentionTarget(const Pubnub::String& target, const Pubnub::String& type);

            Pubnub::String target;
            //TODO: type enum?
            Pubnub::String type;
    };

    PN_CHAT_EXPORT class MessageElement {
        public:
            static MessageElement plain_text(const Pubnub::String& text);
            static MessageElement link(const Pubnub::String& text, const Pubnub::MentionTarget& target);

        private:
            MessageElement(const Pubnub::String& text, const Pubnub::MentionTarget& target);

            Pubnub::String text;
            Pubnub::MentionTarget target;
    };

    PN_CHAT_EXPORT struct SuggestedMention {
        std::size_t offset;
        Pubnub::String replace_from;
        Pubnub::String replace_to;
        Pubnub::MentionTarget target;
    };

    PN_CHAT_EXPORT class MessageDraft
    {
        public:
            PN_CHAT_EXPORT enum class UserSuggestionsSource {
                CHANNELS,
                GLOBAL
            };

            void insert_text(std::size_t position, const Pubnub::String& text);
            void remove_text(std::size_t position, std::size_t length);
            void send(SendTextParams send_params = SendTextParams());


        private:
            MessageDraft(const Pubnub::Channel& channel, const Pubnub::MessageDraftConfig& draft_config);
            void fire_message_elements_changed();

            Pubnub::Channel channel;
            MessageDraftConfig draft_config;
            std::unique_ptr<MessageDraftDAO> value;
            std::unique_ptr<DraftService> draft_service;

            friend ::MessageService;
    };
}
#endif /* PN_CHAT_MESSAGE_DRAFT_H */
