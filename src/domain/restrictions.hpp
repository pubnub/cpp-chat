#ifndef PN_CHAT_RESTRICTIONS_HPP
#define PN_CHAT_RESTRICTIONS_HPP

#include "string.hpp"

// TODO: finish that
class Restrictions {
    Pubnub::String remove_member_payload(const Pubnub::String& user_id);
    Pubnub::String lift_restrictions_payload(const Pubnub::String& channel_id, const Pubnub::String& reason);
    Pubnub::String restrict_member_payload(const Pubnub::String& user_id); 
};

#endif // PN_CHAT_RESTRICTIONS_HPP
