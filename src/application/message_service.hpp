#ifndef PN_CHAT_MESSAGE_SERVICE_HPP
#define PN_CHAT_MESSAGE_SERVICE_HPP

#include "presentation/message.hpp"
#include "export.hpp"
#include "infra/sync.hpp"
#include <memory>

class EntityRepository;
class PubNub;
class MessageEntity;

class MessageService : public std::enable_shared_from_this<MessageService>
{
    public:
        PN_CHAT_EXPORT MessageService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository);


    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;

};

#endif // PN_CHAT_MESSAGE_SERVICE_HPP
