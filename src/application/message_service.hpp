#ifndef PN_CHAT_MESSAGE_SERVICE_HPP
#define PN_CHAT_MESSAGE_SERVICE_HPP

#include "presentation/message.hpp"
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
}

class MessageService : public std::enable_shared_from_this<MessageService>
{
    public:
        MessageService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service);
        Pubnub::ChatMessageData get_message_data(Pubnub::String timetoken);

        void edit_text(Pubnub::Message message, Pubnub::String new_text);
        Pubnub::String text(Pubnub::Message message);
        void delete_message(Pubnub::Message message);
        bool deleted(Pubnub::Message message);

        std::vector<Pubnub::Message> get_channel_history(Pubnub::String channel_id, Pubnub::String start_timetoken, Pubnub::String end_timetoken, int count);
        Pubnub::Message get_message(Pubnub::String timetoken, Pubnub::String channel_id);

        void stream_updates_on(std::vector<Pubnub::Message> messages, std::function<void(Pubnub::Message)> message_callback);

        Pubnub::Message create_message_object(std::pair<Pubnub::String, MessageEntity> message_data);
        Pubnub::Message create_presentation_object(Pubnub::String timetoken);
        
    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

        MessageEntity create_domain_from_presentation_data(Pubnub::String timetoken, Pubnub::ChatMessageData& presentation_data);
        MessageEntity create_domain_from_message_json(Pubnub::String message_json, Pubnub::String channel_id);

        Pubnub::ChatMessageData presentation_data_from_domain(MessageEntity& message_entity);

        void add_message_action_to_message_data(Pubnub::ChatMessageData& message_data, Pubnub::MessageAction& message_action);

};

#endif // PN_CHAT_MESSAGE_SERVICE_HPP
