#include "presentation/channel.hpp"

using namespace Pubnub;

Channel::Channel(std::shared_ptr<ChannelService> channel_service, Pubnub::String channel_id) :
channel_service(channel_service),
channel_id_internal(channel_id)
{}