#include <iostream>
#include "channel.h"


using namespace Pubnub;

void Channel::init(std::string in_channel_id, chat_channel_data in_additional_channel_data)
{
    init(in_channel_id.c_str(), chat_channel_data_c(in_additional_channel_data));
}

void Channel::init(const char* in_channel_id, chat_channel_data_c in_additional_channel_data)
{
    channel_id = in_channel_id;
    channel_data = in_additional_channel_data;

    update(in_additional_channel_data);

    //now channel is fully initialized
    is_initialized = true;
}

void Channel::update(chat_channel_data_c in_additional_channel_data)
{
    channel_data = in_additional_channel_data;
}
