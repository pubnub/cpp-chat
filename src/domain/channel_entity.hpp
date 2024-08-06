#ifndef PN_CHAT_CHANNEL_ENTITY_HPP
#define PN_CHAT_CHANNEL_ENTITY_HPP

#include "domain/json.hpp"
#include "string.hpp"
#include "infra/timer.hpp"
#include "channel.hpp"
#include <map>
#include <utility>


struct ChannelEntity {
    using ChannelId = Pubnub::String;
    using MessageTimetoken = Pubnub::String;

    Pubnub::String channel_name = "";
    Pubnub::String description = "";
    Pubnub::String custom_data_json = "";
    Pubnub::String updated = "";
    Pubnub::String status = "";
    Pubnub::String type = "";

    ChannelEntity pin_message(std::pair<ChannelId, MessageTimetoken> channel_message) const;
    ChannelEntity unpin_message() const;

    //This is equivalent of channel_data_to_json from the old channel obj
    Pubnub::String get_channel_metadata_json_string(Pubnub::String channel_id) const;

    static ChannelEntity from_json(Json channel_json);
    static ChannelEntity from_channel_response(Json response);

    void update_channel_in_stream_channels(Pubnub::Channel channel);

    std::vector<Pubnub::Channel> stream_updates_channels;
};

#endif // PN_CHAT_CHANNEL_ENTITY_HPP
