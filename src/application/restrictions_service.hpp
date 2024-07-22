#ifndef PN_CHAT_RESTRICTIONS_SERVICE_HPP
#define PN_CHAT_RESTRICTIONS_SERVICE_HPP

#include "infra/sync.hpp"
#include "restrictions.hpp"
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

        void set_restrictions(const Pubnub::String& user_id, const Pubnub::String& channel_id, const Pubnub::Restriction& restrictions) const;
        Pubnub::Restriction get_user_restrictions(const Pubnub::String& user_id, const Pubnub::String& channel_id, int limit, const Pubnub::String& start, const Pubnub::String& end) const;
        Pubnub::Restriction get_channel_restrictions(const Pubnub::String& user_id, const Pubnub::String& channel_id, int limit, const Pubnub::String& start, const Pubnub::String& end) const;
        void report_user(const Pubnub::String& user_id, const Pubnub::String& reason) const;
        void report_message(const Pubnub::Message& message, const Pubnub::String& reason) const;

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;
};

#endif // PN_CHAT_RESTRICTIONS_SERVICE_HPP
