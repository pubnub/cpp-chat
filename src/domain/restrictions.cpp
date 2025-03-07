#include "restrictions.hpp"
#include "const_values.hpp"

Pubnub::String Restrictions::remove_member_payload(const Pubnub::String &user_id) {
    return "[{\"uuid\": {\"id\": \"" + user_id + "\"}}]";
}

Pubnub::String Restrictions::lift_restrictions_payload(const Pubnub::String &channel_id, const Pubnub::String &reason) {
    return "{\"channelId\": \"" + channel_id + "\", \"restrictions\": \"lifted\", \"reason\": \"" + reason + "\"}";
}

Pubnub::String Restrictions::restrict_member_payload(const Pubnub::String &user_id) {
    return "[{\"uuid\": {\"id\": \"" + user_id + "\"}}]";
}
