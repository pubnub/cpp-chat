#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include "string.hpp"
#include "export.hpp"

namespace Pubnub
{
    PN_CHAT_EXPORT class Message
    {
        public:

        Pubnub::String timetoken;
        Pubnub::String text;
        Pubnub::String channel_id;
        Pubnub::String user_id;

    };
}
#endif /* MESSAGE_H */
