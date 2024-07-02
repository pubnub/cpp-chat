#ifndef PN_CHAT_CHANNEL_SERVICE_HPP
#define PN_CHAT_CHANNEL_SERVICE_HPP

#include "presentation/channel.hpp"
#include "infra/entity_repository.hpp"
#include "infra/pubnub.hpp"
#include "infra/sync.hpp"
#include "export.hpp"
#include <memory>

class ChannelService : public std::enable_shared_from_this<ChannelService>
{
    public:
        PN_CHAT_EXPORT ChannelService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository);

        Pubnub::Channel create_public_conversation(Pubnub::String channel_id, Pubnub::ChatChannelData data);
        Pubnub::Channel create_channel(Pubnub::String channel_id, Pubnub::ChatChannelData data);
    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;

        //To discuss I think we could change this to return Pubnub::Channel instead of just data
        Pubnub::ChatChannelData presentation_data_from_domain(ChannelEntity& domain_data);
        //To discuss I think we could change this to Pubnub::Channel reference
        ChannelEntity domain_from_presentation_data(Pubnub::ChatChannelData& presentation_data);
};

#endif // PN_CHAT_CHANNEL_SERVICE_HPP
