#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include "export.hpp"

extern "C" {
    #include "core/pubnub_api_types.h"
}


namespace Pubnub
{
    class Chat;
    enum pubnub_chat_message_type : uint8_t;

    struct ChatChannelDataBase
    {
        std::string channel_name;
        std::string description;
        std::string custom_data_json;
        std::string updated;
        int status;
        std::string type;

        ChatChannelDataBase(){};
    };

    struct ChatChannelDataChar
    {
        const char* channel_name;
        const char* description;
        const char* custom_data_json;
        const char* updated;
        int status;
        const char* type;

        ChatChannelDataChar(){};

        ChatChannelDataChar(ChatChannelDataBase in_channel_data)
        {
            channel_name = in_channel_data.channel_name.c_str();
            description = in_channel_data.description.c_str();
            custom_data_json = in_channel_data.custom_data_json.c_str();
            updated = in_channel_data.updated.c_str();
            status = in_channel_data.status;
            type = in_channel_data.type.c_str();
        };
    };
    
    struct ChatChannelData : public ChatChannelDataBase
    {
        ChatChannelData(){};
        ChatChannelData(ChatChannelDataChar in_channel_data)
        {
            channel_name = in_channel_data.channel_name;
            description = in_channel_data.description;
            custom_data_json = in_channel_data.custom_data_json;
            updated = in_channel_data.updated;
            status = in_channel_data.status;
            type = in_channel_data.type;
        };
    };


    class Channel
    {
        public:

        PN_CHAT_EXPORT void init(Pubnub::Chat *InChat, std::string in_channel_id, ChatChannelData in_additional_channel_data);
        PN_CHAT_EXPORT void init(Pubnub::Chat *InChat, const char* in_channel_id, ChatChannelDataChar in_additional_channel_data);
        PN_CHAT_EXPORT void init_from_json(Pubnub::Chat *InChat, std::string in_channel_id, std::string channel_data_json);
        PN_CHAT_EXPORT void init_from_json(Pubnub::Chat *InChat, const char* in_channel_id, const char* channel_data_json);

        PN_CHAT_EXPORT void update(ChatChannelData in_additional_channel_data);
        PN_CHAT_EXPORT void update(ChatChannelDataChar in_additional_channel_data);

        PN_CHAT_EXPORT void connect();
        PN_CHAT_EXPORT void disconnect();

        PN_CHAT_EXPORT void join(std::string additional_params);
        PN_CHAT_EXPORT void Join(const char* additional_params);
        PN_CHAT_EXPORT void leave();
        PN_CHAT_EXPORT void delete_channel();

        PN_CHAT_EXPORT void set_restrictions(std::string in_user_id, bool ban_user, bool mute_user, std::string reason = "");
        PN_CHAT_EXPORT void set_restrictions(const char* in_user_id, bool ban_user, bool mute_user, const char* reason = NULL);

        PN_CHAT_EXPORT void send_text(std::string message, pubnub_chat_message_type message_type, std::string meta_data);
        PN_CHAT_EXPORT void send_text(const char* message, pubnub_chat_message_type message_type, const char* meta_data);

        PN_CHAT_EXPORT std::string get_channel_id(){return channel_id;};
        PN_CHAT_EXPORT ChatChannelData get_channel_data(){return channel_data;};

        private:

        std::string channel_id;
        ChatChannelData channel_data;
        bool is_initialized = false;
        Pubnub::Chat *chat_obj;

        ChatChannelDataChar channel_data_from_json_char(const char* json_char);
        const char* channel_data_to_json_char(const char* channel_id, ChatChannelDataChar channel_data);
        ChatChannelData channel_data_from_json(std::string json_string);
        std::string channel_data_to_json(std::string in_channel_id, ChatChannelData in_channel_data);
        std::string chat_message_to_publish_string(std::string message, pubnub_chat_message_type message_type);

        //Use this to get pubnub publish context from chat_obj.
        pubnub_t* get_ctx_pub();

    };
}
#endif /* CHANNEL_H */
