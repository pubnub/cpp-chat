#ifndef PN_CHAT_PRESENCE_SERVICE_HPP
#define PN_CHAT_PRESENCE_SERVICE_HPP

#include "export.hpp"
#include "infra/sync.hpp"
#include "string.hpp"
#include <memory>
#include <vector>

class EntityRepository;
class PubNub;
class ChatService;


class PresenceService : public std::enable_shared_from_this<PresenceService>
{
    public:
        PN_CHAT_EXPORT PresenceService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service);

        std::vector<Pubnub::String> who_is_present(Pubnub::String channel_id);
        std::vector<Pubnub::String> where_present(Pubnub::String user_id);
        bool is_present(Pubnub::String user_id, Pubnub::String channel_id);

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

};

#endif // PN_CHAT_PRESENCE_SERVICE_HPP
