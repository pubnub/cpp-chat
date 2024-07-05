#ifndef PN_CHAT_MESSAGE_SERVICE_HPP
#define PN_CHAT_MESSAGE_SERVICE_HPP

#include "presentation/message.hpp"
#include "infra/sync.hpp"
#include <memory>

class EntityRepository;
class PubNub;
class ChatService;
class MessageEntity;


class MessageService : public std::enable_shared_from_this<MessageService>
{
    public:
        MessageService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service);

        std::vector<Pubnub::Message> get_channel_history(Pubnub::String channel_id, Pubnub::String start_timetoken, Pubnub::String end_timetoken, int count);
        Pubnub::Message get_message(Pubnub::String timetoken, Pubnub::String channel_id);
        Pubnub::Message create_message_object(std::pair<Pubnub::String, MessageEntity> message_data);

        Pubnub::Message create_presentation_object(Pubnub::String timetoken);
        
    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

        MessageEntity create_domain_from_presentation_data(Pubnub::String timetoken, Pubnub::ChatMessageData& presentation_data);
        MessageEntity create_domain_from_message_json(Pubnub::String message_json, Pubnub::String channel_id);

        Pubnub::ChatMessageData presentation_data_from_domain(MessageEntity& message_entity);

};

#endif // PN_CHAT_MESSAGE_SERVICE_HPP
