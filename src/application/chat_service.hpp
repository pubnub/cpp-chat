#ifndef PN_CHAT_CHAT_SERVICE_HPP
#define PN_CHAT_CHAT_SERVICE_HPP

#include "infra/entity_repository.hpp"
#include "infra/pubnub.hpp"
#include "infra/sync.hpp"
#include "string.hpp"

class ChatService {
    public:
        ChatService(ThreadSafePtr<PubNub> pubnub);

        static ThreadSafePtr<PubNub> create_pubnub(
                Pubnub::String publish_key, Pubnub::String subscribe_key, Pubnub::String user_id);
    private:
        ThreadSafePtr<PubNub> pubnub;
        EntityRepository entity_repository;
};

#endif // PN_CHAT_CHAT_SERVICE_HPP
