#ifndef PN_CHAT_MEMBERSHIP_H
#define PN_CHAT_MEMBERSHIP_H

#include "string.hpp"
#include "export.hpp"
#include "presentation/user.hpp"
#include "presentation/channel.hpp"
#include <memory>
#include <vector>

class ChatService;
class MembershipService;

namespace Pubnub
{

    PN_CHAT_EXPORT class Membership
    {
        public:
            Pubnub::User user;
            Pubnub::Channel channel;

        private:
            Membership(Pubnub::User user, Pubnub::Channel channel, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MembershipService> membership_service);
            std::shared_ptr<ChatService> chat_service;
            std::shared_ptr<MembershipService> membership_service;

            friend class ::MembershipService;
    
    };
}
#endif /* PN_CHAT_MEMBERSHIP_H */
