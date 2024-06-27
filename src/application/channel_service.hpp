#ifndef PN_CHAT_CHANNEL_SERVICE_HPP
#define PN_CHAT_CHANNEL_SERVICE_HPP

#include "presentation/channel.hpp"
#include "infra/entity_repository.hpp"
#include "infra/pubnub.hpp"
#include "infra/sync.hpp"
#include <memory>

class ChannelService {
    public:
        ChannelService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository);

        Pubnub::Channel create_channel(Pubnub::String channel_name, Pubnub::ChatChannelData data);
    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;

        Pubnub::ChatChannelData presentation_data_from_domain(ChannelEntity domain_data);
};

#endif // PN_CHAT_CHANNEL_SERVICE_HPP
