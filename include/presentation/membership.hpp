#ifndef PN_CHAT_MEMBERSHIP_H
#define PN_CHAT_MEMBERSHIP_H

#include "string.hpp"
#include "export.hpp"
#include "presentation/user.hpp"
#include "presentation/channel.hpp"
#include <memory>
#include <vector>
#include <functional>

class ChatService;
class MembershipService;

namespace Pubnub
{
    class Message;

    PN_CHAT_EXPORT class Membership
    {
        public:
            Pubnub::User user;
            Pubnub::Channel channel;

            PN_CHAT_EXPORT Pubnub::String custom_data();

            PN_CHAT_EXPORT Pubnub::Membership update(Pubnub::String custom_object_json);
            PN_CHAT_EXPORT Pubnub::String last_read_message_timetoken();
            PN_CHAT_EXPORT Pubnub::Membership set_last_read_message_timetoken(Pubnub::String timetoken);
            PN_CHAT_EXPORT Pubnub::Membership set_last_read_message(Pubnub::Message message);
            PN_CHAT_EXPORT int get_unread_messages_count(Pubnub::Membership membership);

            PN_CHAT_EXPORT void stream_updates(std::function<void(Pubnub::Membership)> membership_callback);
            PN_CHAT_EXPORT void stream_updates_on(std::vector<Pubnub::Membership> memberships, std::function<void(Pubnub::Membership)> membership_callback);

        private:
            Membership(Pubnub::User user, Pubnub::Channel channel, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MembershipService> membership_service);
            std::shared_ptr<ChatService> chat_service;
            std::shared_ptr<MembershipService> membership_service;

            friend class ::MembershipService;
    
    };
}
#endif /* PN_CHAT_MEMBERSHIP_H */
