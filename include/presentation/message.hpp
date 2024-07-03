#ifndef PN_CHAT_MESSAGE_H
#define PN_CHAT_MESSAGE_H

#include "string.hpp"
#include "export.hpp"
#include <memory>

class MessageService;

namespace Pubnub
{

    class Message
    {
        public:
            PN_CHAT_EXPORT inline Pubnub::String timetoken(){return timetoken_internal;};
            //PN_CHAT_EXPORT Pubnub::String message_data();

        private:
            PN_CHAT_EXPORT Message(std::shared_ptr<MessageService> message_service, Pubnub::String timetoken);
            std::shared_ptr<MessageService> message_service;
            Pubnub::String timetoken_internal;

        friend class MessageService;
    };
}
#endif /* PN_CHAT_MESSAGE_H */
