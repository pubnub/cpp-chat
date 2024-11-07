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
            enum class Type {
                USER,
                CHANNEL, 
                URL
            };

            static MentionTarget user(const Pubnub::String& user_id);
            static MentionTarget channel(const Pubnub::String& channel);
            static MentionTarget url(const Pubnub::String& url);

            Pubnub::String get_target() const;
            Type get_type() const;
        
        private:
            MentionTarget(const Pubnub::String& target, const Type type);

            Pubnub::String target;
            Type type;
    };

    PN_CHAT_EXPORT struct MessageElement {
        public:
            static MessageElement plain_text(const Pubnub::String& text);
            static MessageElement link(const Pubnub::String& text, const Pubnub::MentionTarget& target);

            Pubnub::String text;
            Pubnub::Option<Pubnub::MentionTarget> target;
        private:
            MessageElement(const Pubnub::String& text, const Pubnub::Option<Pubnub::MentionTarget>& target);
    };

    PN_CHAT_EXPORT struct SuggestedMention {
        std::size_t offset;
        Pubnub::String replace_from;
        Pubnub::String replace_to;
        Pubnub::MentionTarget target;
    };

    PN_CHAT_EXPORT class MessageDraft {
        public:
            PN_CHAT_EXPORT enum class UserSuggestionsSource {
                CHANNELS,
                GLOBAL
            };
            ~MessageDraft();

            void insert_text(std::size_t position, const Pubnub::String& text);
            void remove_text(std::size_t position, std::size_t length);
            void insert_suggested_mention(const SuggestedMention& suggested_mention, const Pubnub::String& text);
            void add_mention(std::size_t position, std::size_t length, const Pubnub::MentionTarget& target);
            void remove_mention(std::size_t position);
            void update(const Pubnub::String& text);
            void send(SendTextParams send_params = SendTextParams());

            void add_message_elements_listener(std::function<void(Pubnub::Vector<Pubnub::MessageElement>)> listener);
            void add_message_elements_listener(std::function<void(Pubnub::Vector<Pubnub::MessageElement>, Pubnub::Vector<Pubnub::SuggestedMention>)> listener);

        private:
            MessageDraft(
                    const Pubnub::Channel& channel,
                    const Pubnub::MessageDraftConfig& draft_config,
                    std::shared_ptr<const ChannelService> channel_service,
                    std::shared_ptr<const UserService> user_service);
            void trigger_typing_indicator();

            Pubnub::Channel channel;
            MessageDraftConfig draft_config;
            std::unique_ptr<MessageDraftDAO> value;
            std::unique_ptr<DraftService> draft_service;

            friend ::MessageService;
    };
}
#endif /* PN_CHAT_MESSAGE_DRAFT_H */
