#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include "export.hpp"


namespace Pubnub
{

    struct ChatChannelData
    {
        std::string channel_name;
        std::string description;
        std::string custom_data_json;
        std::string updated;
        std::string status;
        std::string type;
    };

    struct ChatChannelDataChar
    {
        const char* channel_name;
        const char* description;
        const char* custom_data_json;
        const char* updated;
        const char* status;
        const char* type;

        ChatChannelDataChar(){};

        ChatChannelDataChar(ChatChannelData in_channel_data)
        {
            channel_name = in_channel_data.channel_name.c_str();
            description = in_channel_data.description.c_str();
            custom_data_json = in_channel_data.custom_data_json.c_str();
            updated = in_channel_data.updated.c_str();
            status = in_channel_data.status.c_str();
            type = in_channel_data.type.c_str();
        }
    };


    class Channel
    {
        public:

        PN_CHAT_EXPORT void init(std::string in_channel_id, ChatChannelData in_additional_channel_data);
        PN_CHAT_EXPORT void init(const char* in_channel_id, ChatChannelDataChar in_additional_channel_data);
        PN_CHAT_EXPORT void init_from_json(std::string in_channel_id, std::string channel_data_json);
        PN_CHAT_EXPORT void init_from_json(const char* in_channel_id, const char* channel_data_json);

        PN_CHAT_EXPORT void update(ChatChannelData in_additional_channel_data);
        PN_CHAT_EXPORT void update(ChatChannelDataChar in_additional_channel_data);

        PN_CHAT_EXPORT void Connect();
        PN_CHAT_EXPORT void Disconnect();

        private:

        bool is_initialized = false;
        const char* channel_id;
        ChatChannelDataChar channel_data;

        ChatChannelDataChar channel_data_from_json_char(const char* json_char);

    };
}
#endif /* CHANNEL_H */
