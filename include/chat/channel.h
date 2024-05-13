#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include "export.hpp"


namespace Pubnub
{

    PN_CHAT_EXPORT struct chat_channel_data
    {
        std::string channel_name;
        std::string description;
        std::string custom_data_json;
        std::string updated;
        std::string status;
        std::string type;
    };

        PN_CHAT_EXPORT struct chat_channel_data_c
    {
        const char* channel_name;
        const char* description;
        const char* custom_data_json;
        const char* updated;
        const char* status;
        const char* type;

        chat_channel_data_c(){};

        chat_channel_data_c(chat_channel_data in_channel_data)
        {
            channel_name = in_channel_data.channel_name.c_str();
            description = in_channel_data.description.c_str();
            custom_data_json = in_channel_data.custom_data_json.c_str();
            updated = in_channel_data.updated.c_str();
            status = in_channel_data.status.c_str();
            type = in_channel_data.type.c_str();
        }
    };


    PN_CHAT_EXPORT class Channel
    {
        public:

        void init(std::string in_channel_id, chat_channel_data in_additional_channel_data);
        void init(const char* in_channel_id, chat_channel_data_c in_additional_channel_data);

        void update(chat_channel_data in_additional_channel_data);
        void update(chat_channel_data_c in_additional_channel_data);

        private:

        bool is_initialized = false;
        const char* channel_id;
        chat_channel_data_c channel_data;


    };
}
#endif /* CHANNEL_H */
