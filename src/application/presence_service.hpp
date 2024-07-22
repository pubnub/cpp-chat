#ifndef PN_CHAT_PRESENCE_SERVICE_HPP
#define PN_CHAT_PRESENCE_SERVICE_HPP

#include "infra/sync.hpp"
#include "string.hpp"
#include <memory>
#include <vector>
#include <functional>

class EntityRepository;
class PubNub;
class ChatService;


class PresenceService : public std::enable_shared_from_this<PresenceService>
{
    public:
        PresenceService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service);

        std::vector<Pubnub::String> who_is_present(const Pubnub::String& channel_id) const;
        std::vector<Pubnub::String> where_present(const Pubnub::String& user_id) const;
        bool is_present(const Pubnub::String& user_id, const Pubnub::String& channel_id) const;
        void stream_presence(const Pubnub::String& channel_id, std::function<void(const std::vector<Pubnub::String>&)> presence_callback);

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

};

#endif // PN_CHAT_PRESENCE_SERVICE_HPP
