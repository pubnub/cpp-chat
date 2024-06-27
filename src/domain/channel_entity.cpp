#include "channel_entity.hpp"

ChannelEntity::ChannelEntity(Pubnub::String channel_id, ChannelData channel_data):
    channel_id(channel_id),
    channel_data(channel_data)
{}
