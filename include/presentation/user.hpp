#ifndef PN_CHAT_USER_H
#define PN_CHAT_USER_H

#include "string.hpp"
#include "export.hpp"
#include <memory>
#include <vector>

class UserService;
class ChatService;

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
            PN_CHAT_EXPORT Pubnub::ChatUserData user_data();

            PN_CHAT_EXPORT Pubnub::User update(Pubnub::ChatUserData user_data);
            PN_CHAT_EXPORT void delete_user();
            PN_CHAT_EXPORT std::vector<Pubnub::String> where_present();
            PN_CHAT_EXPORT bool is_present_on(Pubnub::String channel_id);

        private:
            PN_CHAT_EXPORT User(Pubnub::String user_id, std::shared_ptr<ChatService> chat_service, std::shared_ptr<UserService> user_service);
            std::shared_ptr<UserService> user_service;
            std::shared_ptr<ChatService> chat_service;
            Pubnub::String user_id_internal;

        friend class UserService;
    };
}
#endif /* PN_CHAT_USER_H */
