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

    struct ChatUserDataBase
    {
        std::string user_name;
        std::string external_id;
        std::string profile_url;
        std::string email;
        std::string custom_data_json;
        int status;
        std::string type;

        ChatUserDataBase(){};
    };

    struct ChatUserDataChar
    {
        const char* user_name;
        const char* external_id;
        const char* profile_url;
        const char* email;
        const char* custom_data_json;
        int status;
        const char* type;

        ChatUserDataChar(){};

        ChatUserDataChar(ChatUserDataBase in_user_data)
        {
            user_name = in_user_data.user_name.c_str();
            external_id = in_user_data.external_id.c_str();
            profile_url = in_user_data.profile_url.c_str();
            email = in_user_data.email.c_str();
            custom_data_json = in_user_data.custom_data_json.c_str();
            status = in_user_data.status;
            type = in_user_data.type.c_str();
        };
    };
    
    struct ChatUserData : public ChatUserDataBase
    {
        ChatUserData(){};
        ChatUserData(ChatUserDataChar in_user_data)
        {
            user_name = in_user_data.user_name;
            external_id = in_user_data.external_id;
            profile_url = in_user_data.profile_url;
            email = in_user_data.email;
            custom_data_json = in_user_data.custom_data_json;
            status = in_user_data.status;
            type = in_user_data.type;
        };
    };

    PN_CHAT_EXPORT class User
    {
        public:

        PN_CHAT_EXPORT void init(Pubnub::Chat *InChat, std::string in_user_id, ChatUserData in_additional_user_data);
        PN_CHAT_EXPORT void init(Pubnub::Chat *InChat, const char* in_user_id, ChatUserDataChar in_additional_user_data);
        PN_CHAT_EXPORT void init_from_json(Pubnub::Chat *InChat, std::string in_user_id, std::string user_data_json);
        PN_CHAT_EXPORT void init_from_json(Pubnub::Chat *InChat, const char* in_user_id, const char* user_data_json);


        private:

        std::string user_id;
        ChatUserData user_data;
        bool is_initialized = false;
        Pubnub::Chat *chat_obj;

        //Use this to get pubnub publish context from chat_obj.
        pubnub_t* get_ctx_pub();

    };
}
#endif /* USER_H */
