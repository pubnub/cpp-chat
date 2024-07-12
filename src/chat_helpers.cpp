#include "chat_helpers.hpp"
#include "nlohmann/json.hpp"


using namespace Pubnub;
using json = nlohmann::json;

namespace Pubnub
{

String create_set_memberships_object(String channel_id, String custom_params_json)
{
    String custom_parameter_string;
    custom_params_json.empty() ? custom_parameter_string="{}" : custom_parameter_string = custom_params_json;
    return String("[{\"channel\": {\"id\": \"") + channel_id +  String("\"}, \"custom\": ") + custom_parameter_string + String("}]");
}

String create_set_members_object(String user_id, String custom_params_json)
{
    String custom_parameter_string;
    custom_params_json.empty() ? custom_parameter_string="{}" : custom_parameter_string = custom_params_json;
    return String("[{\"uuid\": {\"id\": \"") + user_id + String("\"}, \"custom\": ") + custom_parameter_string + String("}]");
}

String create_set_members_object(std::vector<String> users_ids, String custom_params_json)
{
    String custom_parameter_string;
    custom_params_json.empty() ? custom_parameter_string="{}" : custom_parameter_string = custom_params_json;

    //Start json array
    String final_object = "["; 

    //Add all users into that array
    for(auto &user_id : users_ids)
    {
        if(user_id.empty()) {continue;}
        String user_object = String("{\"uuid\": {\"id\": \"") + user_id + String("\"}, \"custom\": ") + custom_parameter_string + String("},");
        final_object += user_object;
    }

    //Erase the last "," and add "]" to close json array
    final_object.erase(final_object.length() - 1, 1);
    final_object += "]";

    return final_object;
}

bool string_starts_with(const String& string, const String prefix)
{
    //If string is smaller it can't start with given prefix
    if (string.length() < prefix.length()) return false;

    return std::equal(prefix.to_std_string().begin(), prefix.to_std_string().end(), string.to_std_string().begin());
}

String chat_message_to_publish_string(String message, pubnub_chat_message_type message_type)
{
    json message_json;
	
	message_json["type"] = chat_message_type_to_string(message_type).c_str();
    message_json["text"] = message.c_str();

	return message_json.dump();
}

}