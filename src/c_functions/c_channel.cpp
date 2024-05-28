#include "c_functions/c_channel.hpp"

void pn_channel_delete(Pubnub::Channel* channel) {
    delete channel;
}

Pubnub::ChatChannelData chat_channel_data_from_helper(ChatChannelDataHelper helper) {
    Pubnub::ChatChannelData data;
    data.channel_name = helper.channel_name;
    data.description = helper.description;
    data.custom_data_json = helper.custom_data_json;
    data.updated = helper.updated;
    data.status = helper.status;
    data.type = helper.type;

    return data;
}
