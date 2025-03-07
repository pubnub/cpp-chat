#ifndef PN_CHAT_MESSAGE_SERVICE_HPP
#define PN_CHAT_MESSAGE_SERVICE_HPP

#include "message.hpp"
#include "infra/sync.hpp"
#include <memory>
#include <vector>
#include <functional>

class EntityRepository;
class PubNub;
class ChatService;
struct MessageEntity;

namespace Pubnub
{
    struct MessageAction;
    class MessageDraft;
    struct MessageDraftConfig;
    class Channel;
    class ThreadMessage;
}

class MessageService : public std::enable_shared_from_this<MessageService>
{
    public:
        MessageService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service);

        Pubnub::Message edit_text(const Pubnub::String& timetoken, const MessageDAO& message_data, const Pubnub::String& new_text) const;
        Pubnub::String text(const MessageDAO& message) const;
        Pubnub::Message delete_message(const MessageDAO& message, const Pubnub::String& timetoken) const;
        bool delete_message_hard(const Pubnub::Message& message) const;
        bool deleted(const MessageDAO& message) const;
        Pubnub::Message restore(const MessageDAO& message, const Pubnub::String& timetoken) const;

        Pubnub::Message get_message(const Pubnub::String& timetoken, const Pubnub::String& channel_id) const;

        std::vector<Pubnub::MessageAction> get_message_reactions(const MessageDAO& message) const;
        Pubnub::Message toggle_reaction(const Pubnub::String& timetoken, const MessageDAO& message, const Pubnub::String& reaction) const;
        void forward_message(const Pubnub::Message& message, const Pubnub::String& channel_id) const;
        Pubnub::MessageDraft create_message_draft(const Pubnub::Channel& channel, const Pubnub::MessageDraftConfig& message_draft_config) const;
        
        std::function<void()> stream_updates(Pubnub::Message calling_message, std::function<void(const Pubnub::Message)> message_callback) const;
        std::function<void()> stream_updates(Pubnub::ThreadMessage calling_message, std::function<void(const Pubnub::ThreadMessage)> message_callback) const;
        std::function<void()> stream_updates_on(Pubnub::Message calling_message, const std::vector<Pubnub::Message>& messages, std::function<void(std::vector<Pubnub::Message>)> message_callback) const;
        std::function<void()> stream_updates_on(Pubnub::ThreadMessage calling_message, const std::vector<Pubnub::ThreadMessage>& messages, std::function<void(std::vector<Pubnub::ThreadMessage>)> message_callback) const;

        Pubnub::Message create_message_object(std::pair<Pubnub::String, MessageEntity> message_data) const;
        Pubnub::ThreadMessage create_thread_message_object(std::pair<Pubnub::String, MessageEntity> message_data, Pubnub::String parent_channel_id) const;
        Pubnub::ThreadMessage create_thread_message_object(const Pubnub::Message& base_message, const Pubnub::String& parent_channel_id) const;

        Pubnub::String get_phrase_to_look_for(const Pubnub::String& look_text) const;
        Pubnub::String get_channel_phrase_to_look_for(const Pubnub::String& look_text) const;

        Pubnub::Message update_message_with_base(const Pubnub::Message& message, const Pubnub::Message& base_message) const;
        Pubnub::ThreadMessage update_thread_message_with_base(const Pubnub::ThreadMessage& message, const Pubnub::ThreadMessage& base_message) const;

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::weak_ptr<const ChatService> chat_service;

        Pubnub::ChatMessageData presentation_data_from_domain(MessageEntity& message_entity);
};

#endif // PN_CHAT_MESSAGE_SERVICE_HPP
