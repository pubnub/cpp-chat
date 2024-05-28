#ifndef USER_H
#define USER_H

#include <string>
#include "export.hpp"

extern "C" {
    #include "core/pubnub_api_types.h"
}

namespace Pubnub
{

    class Chat;

    struct ChatUserData
    {
        Pubnub::String user_name;
        Pubnub::String external_id;
        Pubnub::String profile_url;
        Pubnub::String email;
        Pubnub::String custom_data_json;
        int status;
        Pubnub::String type;

        ChatUserData(){};
    };

    PN_CHAT_EXPORT class User
    {
        public:

        PN_CHAT_EXPORT void init(Pubnub::Chat *InChat, Pubnub::String in_user_id, ChatUserData in_additional_user_data);
        PN_CHAT_EXPORT void init_from_json(Pubnub::Chat *InChat, Pubnub::String in_user_id, Pubnub::String user_data_json);


        private:

        Pubnub::String user_id;
        ChatUserData user_data;
        bool is_initialized = false;
        Pubnub::Chat *chat_obj;

        ChatUserData user_data_from_json(Pubnub::String data_json_string);
        Pubnub::String user_data_to_json(Pubnub::String in_user_id, ChatUserData in_user_data);

        //Use this to get pubnub publish context from chat_obj.
        pubnub_t* get_ctx_pub();

    };
}
#endif /* USER_H */
