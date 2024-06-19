#ifndef CHAT_HELPERS_HPP
#define CHAT_HELPERS_HPP

#include "string.hpp"
#include "extern.hpp"
#include "export.hpp"

namespace Pubnub
{


//Creates Json object required to use set_memberships function.
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::String create_set_memberships_object(Pubnub::String channel_id, Pubnub::String additional_params);

}

#endif /* CHAT_HELPERS_HPP */
