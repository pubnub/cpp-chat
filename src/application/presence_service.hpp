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

        std::vector<Pubnub::String> who_is_present(Pubnub::String channel_id);
        std::vector<Pubnub::String> where_present(Pubnub::String user_id);
        bool is_present(Pubnub::String user_id, Pubnub::String channel_id);
        void stream_presence(Pubnub::String channel_id, std::function<void(std::vector<Pubnub::String>)> presence_callback);

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

};

#endif // PN_CHAT_PRESENCE_SERVICE_HPP
