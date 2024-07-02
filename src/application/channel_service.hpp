#ifndef PN_CHAT_CHANNEL_SERVICE_HPP
#define PN_CHAT_CHANNEL_SERVICE_HPP

#include "presentation/channel.hpp"
#include "export.hpp"
#include "infra/sync.hpp"
#include <memory>

class EntityRepository;
class PubNub;
class ChannelEntity;

class ChannelService : public std::enable_shared_from_this<ChannelService>
{
    public:
        PN_CHAT_EXPORT ChannelService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository);

        Pubnub::Channel create_public_conversation(Pubnub::String channel_id, Pubnub::ChatChannelData data);
        Pubnub::Channel create_channel(Pubnub::String channel_id, Pubnub::ChatChannelData data);
    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;


        ChannelEntity create_domain_from_presentation_data(Pubnub::String channel_id, Pubnub::ChatChannelData& presentation_data);
};

#endif // PN_CHAT_CHANNEL_SERVICE_HPP
