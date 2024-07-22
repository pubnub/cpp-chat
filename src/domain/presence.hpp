#ifndef PN_CHAT_PRESENCE_HPP
#define PN_CHAT_PRESENCE_HPP

#include "json.hpp"
#include "string.hpp"
#include <vector>

namespace Presence {
    std::vector<Pubnub::String> users_from_response(const Json& payload);
    std::vector<Pubnub::String> channels_from_response(const Json& payload);
}

#endif // PN_CHAT_PRESENCE_HPP
