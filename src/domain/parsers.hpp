#ifndef PN_CHAT_PARSERS_HPP
#define PN_CHAT_PARSERS_HPP

#include "domain/channel_entity.hpp"
#include "domain/membership_entity.hpp"
#include "domain/message_entity.hpp"
#include "domain/user_entity.hpp"
#include "string.hpp"
#include "event.hpp"
extern "C" {
#include <pubnub_helper.h>
}

namespace Parsers {
    namespace PubnubJson {
        using Timetoken = Pubnub::String;
        using ChannelId = Pubnub::String;
        using UserId = Pubnub::String;

        bool is_message(Pubnub::String message_json_string);
        bool is_message_update(Pubnub::String message_json_string);
        bool is_channel_update(Pubnub::String message_json_string);
        bool is_user_update(Pubnub::String message_json_string);
        bool is_event(Pubnub::String message_json_string);
        bool is_presence(Pubnub::String message_json_string);
        bool is_membership_update(Pubnub::String message_json_string);

        std::pair<Timetoken, MessageEntity> to_message(pubnub_v2_message pn_message);
        std::pair<ChannelId, ChannelEntity> to_channel(pubnub_v2_message pn_message);
        std::pair<UserId, UserEntity> to_user(pubnub_v2_message pn_message);
        MembershipEntity membership_from_string(Pubnub::String message_json);
        Pubnub::String to_string(pubnub_v2_message pn_message);
        Pubnub::Event to_event(pubnub_v2_message pn_message);

        Pubnub::String event_type(Pubnub::String message_json);
        Pubnub::String message_update_timetoken(Pubnub::String message_json);
        Pubnub::String membership_channel(Pubnub::String message_json);
        Pubnub::String membership_user(Pubnub::String message_json);
        Pubnub::String membership_custom_field(Pubnub::String message_json);
    }
}

#endif // PN_CHAT_PARSERS_HPP
