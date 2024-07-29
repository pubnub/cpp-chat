#ifndef PN_CHAT_CHAT_SERVICE_HPP
#define PN_CHAT_CHAT_SERVICE_HPP

#include "chat.hpp"
#include "infra/sync.hpp"
#include "string.hpp"
#include "enums.hpp"
#include <functional>

class CallbackService;
class ChannelService;
class UserService;
class MessageService;
class PresenceService;
class RestrictionsService;
class MembershipService;
class EntityRepository;
class PubNub;
class AccessManagerService;

class ChatService : public std::enable_shared_from_this<ChatService>
{
    public:
        ChatService(ThreadSafePtr<PubNub> pubnub);
        void init_services(const Pubnub::ChatConfig& config);

        static ThreadSafePtr<PubNub> create_pubnub(const Pubnub::String& publish_key, const Pubnub::String& subscribe_key, const Pubnub::String& user_id);

        void emit_chat_event(Pubnub::pubnub_chat_event_type chat_event_type, const Pubnub::String& channel_id, const Pubnub::String& payload) const;
#ifndef PN_CHAT_C_ABI
        void listen_for_events(const Pubnub::String& channel_id, Pubnub::pubnub_chat_event_type chat_event_type, std::function<void(const Pubnub::String&)> event_callback) const;
#endif

        std::shared_ptr<const ChannelService> channel_service;
        std::shared_ptr<const UserService> user_service;
        std::shared_ptr<const MessageService> message_service;
        std::shared_ptr<const MembershipService> membership_service;
        std::shared_ptr<const PresenceService> presence_service;
        std::shared_ptr<const RestrictionsService> restrictions_service;
        std::shared_ptr<const AccessManagerService> access_manager_service;
#ifndef PN_CHAT_C_ABI
        std::shared_ptr<CallbackService> callback_service;
#endif
    
    private:
        ThreadSafePtr<PubNub> pubnub;

#ifdef PN_CHAT_C_ABI
        std::vector<Pubnub::String> listen_for_events(const Pubnub::String& channel_id, Pubnub::pubnub_chat_event_type chat_event_type) const;
#endif
};

#endif // PN_CHAT_CHAT_SERVICE_HPP
