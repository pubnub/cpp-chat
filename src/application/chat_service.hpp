#ifndef PN_CHAT_CHAT_SERVICE_HPP
#define PN_CHAT_CHAT_SERVICE_HPP

#include "infra/sync.hpp"
#include "string.hpp"
#include "export.hpp"

class ChannelService;
class UserService;
class MessageService;
class EntityRepository;
class PubNub;

class ChatService {
    public:
        PN_CHAT_EXPORT ChatService(ThreadSafePtr<PubNub> pubnub);

        PN_CHAT_EXPORT static ThreadSafePtr<PubNub> create_pubnub(Pubnub::String publish_key, Pubnub::String subscribe_key, Pubnub::String user_id);

        std::shared_ptr<ChannelService> channel_service;
        std::shared_ptr<UserService> user_service;
        std::shared_ptr<MessageService> message_service;
    
    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        
};

#endif // PN_CHAT_CHAT_SERVICE_HPP
