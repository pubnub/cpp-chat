#ifndef USER_H
#define USER_H

#include "export.hpp"
#include "string.hpp"
#include <functional>
#include <vector>

namespace Pubnub
{
    struct PubnubRestrictionsData;
    class Chat;
    class Membership;

    struct ChatUserData
    {
        Pubnub::String user_name;
        Pubnub::String external_id;
        Pubnub::String profile_url;
        Pubnub::String email;
        Pubnub::String custom_data_json;
        Pubnub::String status;
        Pubnub::String type;
    };

    PN_CHAT_EXPORT class User
    {
    public:
        PN_CHAT_EXPORT User(Pubnub::Chat& chat, Pubnub::String user_id, ChatUserData additional_user_data);
        PN_CHAT_EXPORT User(Pubnub::Chat& chat, Pubnub::String user_id, Pubnub::String user_data_json);
        //From channel member json
        PN_CHAT_EXPORT User(Pubnub::Chat& chat, Pubnub::String user_data_json);

        ~User() = default;

        PN_CHAT_EXPORT Pubnub::User update(ChatUserData in_user_data);
        PN_CHAT_EXPORT void delete_user();
        PN_CHAT_EXPORT void set_restrictions(Pubnub::String in_channel_id, Pubnub::PubnubRestrictionsData restrictions);
        PN_CHAT_EXPORT void report(Pubnub::String reason);
        PN_CHAT_EXPORT std::vector<Pubnub::String> where_present();
        PN_CHAT_EXPORT bool is_present_on(Pubnub::String channel_id);
        PN_CHAT_EXPORT std::vector<Pubnub::Membership> get_memberships(int limit, Pubnub::String start_timetoken, Pubnub::String end_timetoken);
        PN_CHAT_EXPORT PubnubRestrictionsData get_channel_restrictions(Pubnub::String in_user_id, Pubnub::String in_channel_id, int limit, String start, String end);

        PN_CHAT_EXPORT void stream_updates(std::function<void(User)> user_callback);
        PN_CHAT_EXPORT void stream_updates_on(std::vector<Pubnub::User> users, std::function<void(User)> user_callback);

        ChatUserData user_data_from_json(Pubnub::String data_json_string);
        Pubnub::String user_id_from_json(Pubnub::String data_json_string);
        Pubnub::String user_data_to_json(Pubnub::String in_user_id, ChatUserData in_user_data);

        PN_CHAT_EXPORT Pubnub::String get_user_id();
        PN_CHAT_EXPORT ChatUserData get_user_data();

    private:

        Pubnub::String user_id;
        ChatUserData user_data;
        bool is_initialized = false;
        Pubnub::Chat& chat_obj;
    };
}
#endif /* USER_H */
