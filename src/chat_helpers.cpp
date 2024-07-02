#include "chat_helpers.hpp"


namespace Pubnub
{

Pubnub::String create_set_memberships_object(Pubnub::String channel_id, Pubnub::String custom_params_json)
{
    Pubnub::String custom_parameter_string;
    custom_params_json.empty() ? custom_parameter_string="{}" : custom_parameter_string = custom_params_json;
    return Pubnub::String("[{\"channel\": {\"id\": \"") + channel_id +  Pubnub::String("\"}, \"custom\": ") + custom_parameter_string + Pubnub::String("}]");
}

Pubnub::String create_set_members_object(Pubnub::String user_id, Pubnub::String custom_params_json)
{
    Pubnub::String custom_parameter_string;
    custom_params_json.empty() ? custom_parameter_string="{}" : custom_parameter_string = custom_params_json;
    return Pubnub::String("[{\"uuid\": {\"id\": \"") + user_id + Pubnub::String("\"}, \"custom\": ") + custom_parameter_string + Pubnub::String("}]");
}

Pubnub::String create_set_members_object(std::vector<Pubnub::String> users_ids, Pubnub::String custom_params_json)
{
    Pubnub::String custom_parameter_string;
    custom_params_json.empty() ? custom_parameter_string="{}" : custom_parameter_string = custom_params_json;

    //Start json array
    Pubnub::String final_object = "["; 

    //Add all users into that array
    for(auto &user_id : users_ids)
    {
        if(user_id.empty()) {continue;}
        Pubnub::String user_object = Pubnub::String("{\"uuid\": {\"id\": \"") + user_id + Pubnub::String("\"}, \"custom\": ") + custom_parameter_string + Pubnub::String("},");
        final_object += user_object;
    }

    //Erase the last "," and add "]" to close json array
    final_object.erase(final_object.length() - 1, 1);
    final_object += "]";

    return final_object;
}

bool string_starts_with(const Pubnub::String& string, const Pubnub::String prefix)
{
    //If string is smaller it can't start with given prefix
    if (string.length() < prefix.length()) return false;

    return std::equal(prefix.to_std_string().begin(), prefix.to_std_string().end(), string.to_std_string().begin());
}

}