#ifndef CHAT_HELPERS_HPP
#define CHAT_HELPERS_HPP

#include "string.hpp"
#include "extern.hpp"
#include "export.hpp"
#include <vector>

namespace Pubnub
{


//Creates Json object required to use set_memberships function.
Pubnub::String create_set_memberships_object(Pubnub::String channel_id, Pubnub::String custom_params_json);

//Creates Json object required to use set_members function.
Pubnub::String create_set_members_object(Pubnub::String user_id, Pubnub::String custom_params_json);

//Creates Json object required to use set_members function. Overload for multiple users
Pubnub::String create_set_members_object(std::vector<Pubnub::String> users_ids, Pubnub::String custom_params_json);

}

#endif /* CHAT_HELPERS_HPP */
