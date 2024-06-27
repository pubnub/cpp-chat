#ifndef PN_CHAT_CHANNEL_ENTITY_HPP
#define PN_CHAT_CHANNEL_ENTITY_HPP

#include "string.hpp"

class ChannelEntity {
    public:
        struct ChannelData {
            Pubnub::String channel_name;
            Pubnub::String description;
            Pubnub::String custom_data_json;
            Pubnub::String updated;
            Pubnub::String status;
            Pubnub::String type;
        };

        ChannelEntity(Pubnub::String channel_id, ChannelData channel_data);
    private:
        Pubnub::String channel_id;
        ChannelData channel_data;
};

#endif // PN_CHAT_CHANNEL_ENTITY_HPP
