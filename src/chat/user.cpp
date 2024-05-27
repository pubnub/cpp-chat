#include <iostream>
#include "nlohmann/json.hpp"
#include "chat.hpp"

#include "chat/user.hpp"

extern "C" {
#include "core/pubnub_objects_api.h"
}

using namespace Pubnub;

void User::init(Pubnub::Chat *InChat, std::string in_user_id, ChatUserData in_additional_user_data)
{
    chat_obj = InChat;
    user_id = in_user_id;
    user_data = in_additional_user_data;

    //pubnub_set_uuidmetadata(get_ctx_pub(), in_user_id.c_str(), NULL, channel_data_to_json(channel_id, channel_data).c_str());

    //now channel is fully initialized
    is_initialized = true;
}

void User::init(Pubnub::Chat *InChat, const char *in_user_id, ChatUserDataChar in_additional_user_data)
{

}

void User::init_from_json(Pubnub::Chat *InChat, std::string in_user_id, std::string user_data_json)
{

}

void User::init_from_json(Pubnub::Chat *InChat, const char *in_user_id, const char *user_data_json)
{

}

pubnub_t* User::get_ctx_pub()
{
    if(!chat_obj)
    //TODO: throw exception, error or sth
    {return nullptr;}
    return chat_obj->get_pubnub_context();
}