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

        Pubnub::String message_text = "initial message text";
        int test_int = 1500;
        Pubnub::String message_id;
        int test_int2 = 88888;
    };
}
#endif /* MESSAGE_H */
