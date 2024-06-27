#include "channel_service.hpp"
#include "infra/pubnub.hpp"

ChannelService::ChannelService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository):
    pubnub(pubnub),
    entity_repository(entity_repository)
{}

Pubnub::Channel ChannelService::create_channel(Pubnub::String channel_name, Pubnub::ChatChannelData data) {
    auto maybe_channel = this->entity_repository->get_channel_entities().get(channel_name);

    if (maybe_channel.has_value()) {
        return Pubnub::Channel(channel_name, this->presentation_data_from_domain(maybe_channel.value()));
    }

    auto channel = Pubnub::Channel(channel_name, data);

    // TODO: jsonize data, push to pubnub, save to repository

    return channel;
}

