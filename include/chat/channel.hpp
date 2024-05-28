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

    struct ChatChannelData
    {
        Pubnub::String channel_name;
        Pubnub::String description;
        Pubnub::String custom_data_json;
        Pubnub::String updated;
        int status;
        Pubnub::String type;

        ChatChannelData(){};
    };


    class Channel
    {
        public:

        PN_CHAT_EXPORT void init(Pubnub::Chat *InChat, Pubnub::String in_channel_id, ChatChannelData in_additional_channel_data);
        PN_CHAT_EXPORT void init_from_json(Pubnub::Chat *InChat, Pubnub::String in_channel_id, Pubnub::String channel_data_json);

        PN_CHAT_EXPORT void update(ChatChannelData in_additional_channel_data);

        PN_CHAT_EXPORT void connect();
        PN_CHAT_EXPORT void disconnect();

        PN_CHAT_EXPORT void join(Pubnub::String additional_params);
        PN_CHAT_EXPORT void leave();
        PN_CHAT_EXPORT void delete_channel();

        PN_CHAT_EXPORT void set_restrictions(Pubnub::String in_user_id, bool ban_user, bool mute_user, Pubnub::String reason = "");

        PN_CHAT_EXPORT void send_text(Pubnub::String message, pubnub_chat_message_type message_type, Pubnub::String meta_data);

        PN_CHAT_EXPORT Pubnub::String get_channel_id(){return channel_id;};
        PN_CHAT_EXPORT ChatChannelData get_channel_data(){return channel_data;};

        private:

        Pubnub::String channel_id;
        ChatChannelData channel_data;
        bool is_initialized = false;
        Pubnub::Chat *chat_obj;

        ChatChannelData channel_data_from_json(Pubnub::String json_string);
        Pubnub::String channel_data_to_json(Pubnub::String in_channel_id, ChatChannelData in_channel_data);
        Pubnub::String chat_message_to_publish_string(Pubnub::String message, pubnub_chat_message_type message_type);

        //Use this to get pubnub publish context from chat_obj.
        pubnub_t* get_ctx_pub();

    };
}
#endif /* CHANNEL_H */
