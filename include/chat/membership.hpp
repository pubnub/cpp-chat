#ifndef MEMBERSHIP_H
#define MEMBERSHIP_H

#include <string>
#include <vector>
#include "string.hpp"
#include "export.hpp"
#include "chat.hpp"


namespace Pubnub
{
    class User;
    class Channel;
    class Chat;

    PN_CHAT_EXPORT class Membership
    {
        public:
        //From user membership
        PN_CHAT_EXPORT Membership(Pubnub::Chat& in_chat, Pubnub::User in_user, Pubnub::String in_membership_json);
        //From channel member
        PN_CHAT_EXPORT Membership(Pubnub::Chat& in_chat, Pubnub::Channel in_channel, Pubnub::String in_member_json);

        PN_CHAT_EXPORT Membership(Pubnub::Chat& in_chat, Pubnub::Channel in_channel, Pubnub::User in_user, Pubnub::String in_custom_json);

        ~Membership() = default;

        PN_CHAT_EXPORT void stream_updates(std::function<void(Membership)> membership_callback);
        PN_CHAT_EXPORT void stream_updates_on(std::vector<Pubnub::Membership> memberships, std::function<void(Membership)> membership_callback);
        
        PN_CHAT_EXPORT Membership update(Pubnub::String in_custom_object_json);

        PN_CHAT_EXPORT Pubnub::String get_user_id();
        PN_CHAT_EXPORT Pubnub::String get_channel_id();
        PN_CHAT_EXPORT Pubnub::String get_custom_data_json();

        private:
        Pubnub::Chat& chat_obj;
        Pubnub::String custom_data_json;
        
        Pubnub::User user;
        Pubnub::Channel channel;


    };
}
#endif /* MEMBERSHIP_H */
