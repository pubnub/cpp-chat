#include "chat/chat_helpers.hpp"


namespace Pubnub
{

Pubnub::String create_set_memberships_object(Pubnub::String channel_id, Pubnub::String additional_params)
{
    String custom_parameter_string;
    additional_params.empty() ? custom_parameter_string="{}" : custom_parameter_string = additional_params;
    return Pubnub::String("[{\"channel\": {\"id\": \"") + channel_id +  Pubnub::String("\"}, \"custom\": ") + custom_parameter_string + Pubnub::String("}]");
}

}