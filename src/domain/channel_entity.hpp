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
};

#endif // PN_CHAT_CHANNEL_ENTITY_HPP
