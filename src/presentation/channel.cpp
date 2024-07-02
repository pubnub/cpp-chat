#include "presentation/channel.hpp"

using namespace Pubnub;

Channel::Channel(std::shared_ptr<ChannelService> channel_service, String channel_id, ChatChannelData channel_data) :
channel_service(channel_service)
{}