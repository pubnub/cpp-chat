#include <iostream>
#include "channel.h"

extern "C" {
#include "core/pubnub_objects_api.h"
}

using namespace Pubnub;

void Channel::init(std::string in_channel_id, ChatChannelData in_additional_channel_data)
{
    //init(in_channel_id.c_str(), ChatChannelDataChar(in_additional_channel_data));
}

void Channel::init(const char* in_channel_id, ChatChannelDataChar in_additional_channel_data)
{
    channel_id = in_channel_id;
    channel_data = in_additional_channel_data;

    update(in_additional_channel_data);

    //now channel is fully initialized
    is_initialized = true;
}

void Channel::update(ChatChannelData in_additional_channel_data)
{
    update(ChatChannelDataChar(in_additional_channel_data));
}

void Channel::update(ChatChannelDataChar in_additional_channel_data)
{
    channel_data = in_additional_channel_data;

    //pubnub_set_channelmetadata();


}
