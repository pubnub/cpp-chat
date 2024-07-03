#ifndef PN_CHAT_USER_H
#define PN_CHAT_USER_H

#include "string.hpp"
#include "export.hpp"
#include <memory>

class UserService;

namespace Pubnub
{
    struct ChatUserData
    {
        Pubnub::String user_name = "";
        Pubnub::String external_id = "";
        Pubnub::String profile_url = "";
        Pubnub::String email = "";
        Pubnub::String custom_data_json = "";
        Pubnub::String status = "";
        Pubnub::String type = "";
    };


    class User
    {
        public:
            PN_CHAT_EXPORT inline Pubnub::String user_id(){return user_id_internal;};
            PN_CHAT_EXPORT Pubnub::String channel_data();

        private:
            PN_CHAT_EXPORT User(std::shared_ptr<UserService> user_service, Pubnub::String user_id);
            std::shared_ptr<UserService> user_service;
            Pubnub::String user_id_internal;

        friend class UserService;
    };
}
#endif /* PN_CHAT_USER_H */
