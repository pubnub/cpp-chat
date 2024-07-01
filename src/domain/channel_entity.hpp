#ifndef PN_CHAT_CHANNEL_ENTITY_HPP
#define PN_CHAT_CHANNEL_ENTITY_HPP

#include "string.hpp"

struct ChannelEntity {
    Pubnub::String channel_name;
    Pubnub::String description;
    Pubnub::String custom_data_json;
    Pubnub::String updated;
    Pubnub::String status;
    Pubnub::String type;

    //This is equivalent of channel_data_to_json from the old channel obj
    Pubnub::String get_channel_metadata_json_string();
};

#endif // PN_CHAT_CHANNEL_ENTITY_HPP
