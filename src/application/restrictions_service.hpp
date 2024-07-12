#ifndef PN_CHAT_RESTRICTIONS_SERVICE_HPP
#define PN_CHAT_RESTRICTIONS_SERVICE_HPP

#include "infra/sync.hpp"
#include "presentation/restrictions.hpp"
#include "string.hpp"
#include <memory>
#include <vector>

class EntityRepository;
class PubNub;
class ChatService;

namespace Pubnub
{
    class Message;
}


class RestrictionsService : public std::enable_shared_from_this<RestrictionsService>
{
    public:
        RestrictionsService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service);

        void set_restrictions(Pubnub::String user_id, Pubnub::String channel_id, Pubnub::Restriction restrictions);
        Pubnub::Restriction get_user_restrictions(Pubnub::String user_id, Pubnub::String channel_id, int limit, Pubnub::String start, Pubnub::String end);
        Pubnub::Restriction get_channel_restrictions(Pubnub::String user_id, Pubnub::String channel_id, int limit, Pubnub::String start, Pubnub::String end);
        void report_user(Pubnub::String user_id, Pubnub::String reason);
        void report_message(Pubnub::Message message, Pubnub::String reason);

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;
};

#endif // PN_CHAT_RESTRICTIONS_SERVICE_HPP