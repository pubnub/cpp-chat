#include "presentation/channel.hpp"
#include "application/channel_service.hpp"

using namespace Pubnub;

Channel::Channel(std::shared_ptr<ChannelService> channel_service, Pubnub::String channel_id) :
channel_service(channel_service),
channel_id_internal(channel_id)
{}

Pubnub::ChatChannelData Channel::channel_data()
{
    return channel_service->get_channel_data(channel_id_internal);
}