#ifndef PN_CHAT_CHAT_SERVICE_HPP
#define PN_CHAT_CHAT_SERVICE_HPP

#include "chat.hpp"
#include "infra/sync.hpp"
#include "mentions.hpp"
#include "string.hpp"
#include "enums.hpp"
#include <functional>
#include <vector>
#ifdef PN_CHAT_C_ABI
#include <pubnub_helper.h>
#endif

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


#ifdef PN_CHAT_C_ABI
struct pubnub_v2_message;
#endif

class ChatService : public std::enable_shared_from_this<ChatService>
{
    public:
        ChatService(ThreadSafePtr<PubNub> pubnub);
        void init_services(const Pubnub::ChatConfig& config);

        static ThreadSafePtr<PubNub> create_pubnub(const Pubnub::String& publish_key, const Pubnub::String& subscribe_key, const Pubnub::String& user_id, const Pubnub::String& auth_key);

        void emit_chat_event(Pubnub::pubnub_chat_event_type chat_event_type, const Pubnub::String& channel_id, const Pubnub::String& payload, Pubnub::EventMethod event_method = Pubnub::EventMethod::Default) const;
        std::tuple<std::vector<Pubnub::Event>, bool> get_events_history(const Pubnub::String& channel_id, const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, int count) const;
        std::tuple<std::vector<Pubnub::UserMentionData>, bool> get_current_user_mentions(const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, int count) const;
#ifndef PN_CHAT_C_ABI
        std::function<void()> listen_for_events(const Pubnub::String& channel_id, Pubnub::pubnub_chat_event_type chat_event_type, std::function<void(const Pubnub::Event&)> event_callback) const;
#endif

        std::shared_ptr<const UserService> user_service;
        std::shared_ptr<const MessageService> message_service;
        std::shared_ptr<const MembershipService> membership_service;
        std::shared_ptr<const PresenceService> presence_service;
        std::shared_ptr<const RestrictionsService> restrictions_service;
        std::shared_ptr<const AccessManagerService> access_manager_service;
        std::shared_ptr<const ChannelService> channel_service;
        Pubnub::ChatConfig chat_config;

        std::shared_ptr<CallbackService> callback_service;

#ifdef PN_CHAT_C_ABI
        std::vector<pubnub_v2_message> listen_for_events(const Pubnub::String& channel_id, Pubnub::pubnub_chat_event_type chat_event_type) const;
        std::vector<pubnub_v2_message> get_chat_updates() const;
#endif
    
    private:
        ThreadSafePtr<PubNub> pubnub;
};

#endif // PN_CHAT_CHAT_SERVICE_HPP
