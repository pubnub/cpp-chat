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

    PN_CHAT_EXPORT class Memebership
    {
        public:
        //From user membership
        PN_CHAT_EXPORT Memebership(Pubnub::Chat& in_chat, Pubnub::User& in_user, Pubnub::String in_membership_json);
        //From channel member
        PN_CHAT_EXPORT Memebership(Pubnub::Chat& in_chat, Pubnub::Channel& in_channel, Pubnub::String in_member_json);

        ~Memebership() = default;

        private:
        Pubnub::Chat& chat_obj;
        Pubnub::User& user;
        Pubnub::Channel& channel;
        Pubnub::String custom_data_json;


    };
}
#endif /* MEMBERSHIP_H */
