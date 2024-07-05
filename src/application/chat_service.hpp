#ifndef PN_CHAT_CHAT_SERVICE_HPP
#define PN_CHAT_CHAT_SERVICE_HPP

#include "infra/sync.hpp"
#include "string.hpp"
#include "enums.hpp"
#include <functional>

class ChannelService;
class UserService;
class MessageService;
class PresenceService;
class RestrictionsService;
class MembershipService;
class EntityRepository;
class PubNub;

class ChatService : public std::enable_shared_from_this<ChatService>
{
    public:
        ChatService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository);
        void init_services();

        static ThreadSafePtr<PubNub> create_pubnub(Pubnub::String publish_key, Pubnub::String subscribe_key, Pubnub::String user_id);
        static std::shared_ptr<EntityRepository> create_entity_repository();

        void emit_chat_event(Pubnub::pubnub_chat_event_type chat_event_type, Pubnub::String channel_id, Pubnub::String payload);
        void listen_for_events(Pubnub::String channel_id, Pubnub::pubnub_chat_event_type chat_event_type, std::function<void(Pubnub::String)> event_callback);

        std::shared_ptr<ChannelService> channel_service;
        std::shared_ptr<UserService> user_service;
        std::shared_ptr<MessageService> message_service;
        std::shared_ptr<MembershipService> membership_service;
        std::shared_ptr<PresenceService> presence_service;
        std::shared_ptr<RestrictionsService> restrictions_service;
    
    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        
};

#endif // PN_CHAT_CHAT_SERVICE_HPP
