#ifndef PN_CHAT_BUNDLES_HPP
#define PN_CHAT_BUNDLES_HPP

#include <memory>
#include "channel_service.hpp"
#include "membership_service.hpp"
#include "user_service.hpp"
#include "chat_service.hpp"
#include "message_service.hpp"

struct EntityServicesBundle {
    std::weak_ptr<ChannelService> channel_service;
    std::weak_ptr<UserService> user_service;
    std::weak_ptr<ChatService> chat_service;
    std::weak_ptr<MessageService> message_service;
    std::weak_ptr<MembershipService> membership_service;
};

#endif // PN_CHAT_BUNDLES_HPP
