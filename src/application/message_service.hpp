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


    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

};

#endif // PN_CHAT_MESSAGE_SERVICE_HPP
