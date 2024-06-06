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

        ~Membership() = default;

        private:
        Pubnub::Chat& chat_obj;
        Pubnub::String custom_data_json;
        
        Pubnub::User user;
        Pubnub::Channel channel;


    };
}
#endif /* MEMBERSHIP_H */
