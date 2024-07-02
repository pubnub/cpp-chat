#include "channel_service.hpp"
#include "infra/pubnub.hpp"

ChannelService::ChannelService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository):
    pubnub(pubnub),
    entity_repository(entity_repository)
{}

Pubnub::Channel ChannelService::create_public_conversation(Pubnub::String channel_id, Pubnub::ChatChannelData data)
{
    data.type = "public";
    return create_channel(channel_id, data);
}


Pubnub::Channel ChannelService::create_channel(Pubnub::String channel_id, Pubnub::ChatChannelData data) {
    auto maybe_channel = this->entity_repository->get_channel_entities().get(channel_id);

    if (maybe_channel.has_value()) {
        return Pubnub::Channel(shared_from_this(), channel_id, this->presentation_data_from_domain(maybe_channel.value()));
    }

    /* To domain
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to create channel, channel_id is empty");
    }*/

    auto channel = Pubnub::Channel(shared_from_this(), channel_id, data);

    //This should be moved to infra
    //this->pubnub.set_channel_metadata(channel_id, channel.get_channel_metadata_json_string());

    ChannelEntity channel_entity = domain_from_presentation_data(data);
    
    //Add channel_entity to repository
    
    return channel;
}

Pubnub::ChatChannelData ChannelService::presentation_data_from_domain(ChannelEntity &domain_data)
{
    return Pubnub::ChatChannelData();
}

ChannelEntity ChannelService::domain_from_presentation_data(Pubnub::ChatChannelData &presentation_data)
{
    return ChannelEntity();
}
