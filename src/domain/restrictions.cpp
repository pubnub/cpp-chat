#include "restrictions.hpp"
#include "const_values.hpp"

Pubnub::String Restrictions::remove_member_payload(const Pubnub::String &user_id) {
    return "[{\"uuid\": {\"id\": \"" + user_id + "\"}}]";
}

Pubnub::String Restrictions::lift_restrictions_payload(const Pubnub::String &channel_id, const Pubnub::String &reason) {
    auto restrictions_channel = Pubnub::INTERNAL_MODERATION_PREFIX + channel_id;
    return "{\"channelId\": \"" + restrictions_channel + "\", \"restrictions\": \"lifted\", \"reason\": \"" + reason + "\"}";
}
