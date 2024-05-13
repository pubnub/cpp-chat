#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include "export.hpp"


namespace Pubnub
{

    PN_CHAT_EXPORT struct pubnub_chat_channel_data
    {
        std::string channel_name;
        std::string description;
        std::string custom_data_json;
        std::string updated;
        std::string status;
        std::string type;
    };

    PN_CHAT_EXPORT class Channel
    {
        public:

        void init(std::string channel_id, pubnub_chat_channel_data additional_channel_data);

    };
}
#endif /* CHANNEL_H */
