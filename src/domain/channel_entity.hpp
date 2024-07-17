#ifndef PN_CHAT_CHANNEL_ENTITY_HPP
#define PN_CHAT_CHANNEL_ENTITY_HPP

#include "domain/json.hpp"
#include "string.hpp"
#include "infra/timer.hpp"
#include <map>

struct ChannelEntity {
    Pubnub::String channel_name = "";
    Pubnub::String description = "";
    Pubnub::String custom_data_json = "";
    Pubnub::String updated = "";
    Pubnub::String status = "";
    Pubnub::String type = "";

    bool typing_sent = false;
    Timer typing_sent_timer;
    std::map<Pubnub::String, Timer, Pubnub::StringComparer> typing_indicators;

    inline void set_typing_sent(bool value) {typing_sent = value;};
    inline void set_typing_sent_timer(Timer value) {typing_sent_timer = value;};

    //This is equivalent of channel_data_to_json from the old channel obj
    Pubnub::String get_channel_metadata_json_string(Pubnub::String channel_id);
    static ChannelEntity from_json(Json channel_json);
};

#endif // PN_CHAT_CHANNEL_ENTITY_HPP
