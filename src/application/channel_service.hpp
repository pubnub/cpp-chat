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

        Pubnub::Channel create_public_conversation(Pubnub::String channel_id, Pubnub::ChannelData data);
        Pubnub::Channel create_channel(Pubnub::String channel_id, Pubnub::ChannelData data);
    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;

        //To discuss I think we could change this to return Pubnub::Channel instead of just data
        Pubnub::ChannelData presentation_data_from_domain(ChannelEntity& domain_data);
        //To discuss I think we could change this to Pubnub::Channel reference
        ChannelEntity domain_from_presentation_data(Pubnub::ChannelData& presentation_data);
};

#endif // PN_CHAT_CHANNEL_SERVICE_HPP
