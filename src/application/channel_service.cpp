#include "channel_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"

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

    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to create channel, channel_id is empty");
    }

    auto maybe_channel = this->entity_repository->get_channel_entities().get(channel_id);

    if (maybe_channel.has_value()) {
        return Pubnub::Channel(shared_from_this(), channel_id);
    }

    auto channel = Pubnub::Channel(shared_from_this(), channel_id);

    ChannelEntity new_channel_entity = create_domain_from_presentation_data(channel_id, data);

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->set_channel_metadata(channel_id, new_channel_entity.get_channel_metadata_json_string());

    //Add channel_entity to repository
    entity_repository->get_channel_entities().update_or_insert(channel_id, new_channel_entity);
    
    return channel;
}

ChannelEntity ChannelService::create_domain_from_presentation_data(Pubnub::String channel_id, Pubnub::ChatChannelData &presentation_data)
{
    ChannelEntity new_channel_entity;
    new_channel_entity.channel_id = channel_id;
    new_channel_entity.channel_name = presentation_data.channel_name;
    new_channel_entity.description = presentation_data.description;
    new_channel_entity.custom_data_json = presentation_data.custom_data_json;
    new_channel_entity.updated = presentation_data.updated;
    new_channel_entity.status = presentation_data.status;
    new_channel_entity.type = presentation_data.type;

    return new_channel_entity;
}
