#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include <vector>
#include "export.hpp"
#include "enums.hpp"
#include "infra/pubnub.hpp"
#include "chat.hpp"
#include "string.hpp"
#include "callbacks.hpp"

extern "C" {
    #include "core/pubnub_api_types.h"
}


namespace Pubnub
{
    class Chat;
    class Message;
    class Membership;
    struct PubnubRestrictionsData;

    struct ChatChannelData
    {
        Pubnub::String channel_name;
        Pubnub::String description;
        Pubnub::String custom_data_json;
        Pubnub::String updated;
        Pubnub::String status;
        Pubnub::String type;

        ChatChannelData(){};
    };


    class Channel
    {
        public:

        PN_CHAT_EXPORT Channel(Pubnub::Chat& InChat, Pubnub::String in_channel_id, ChatChannelData in_additional_channel_data);
        PN_CHAT_EXPORT Channel(Pubnub::Chat& InChat, Pubnub::String in_channel_id, Pubnub::String channel_data_json);
        //From user membership Json
        PN_CHAT_EXPORT Channel(Pubnub::Chat& InChat, Pubnub::String channel_data_json);

        PN_CHAT_EXPORT void update(ChatChannelData in_additional_channel_data);
        PN_CHAT_EXPORT void connect(std::function<void(Message)> message_callback);
        PN_CHAT_EXPORT void connect(CallbackStringFunction string_callback);
        PN_CHAT_EXPORT void disconnect();
        PN_CHAT_EXPORT void join(std::function<void(Message)> message_callback, Pubnub::String additional_params = "");
        PN_CHAT_EXPORT void join(CallbackStringFunction string_callback, Pubnub::String additional_params = "");
        PN_CHAT_EXPORT void leave();
        PN_CHAT_EXPORT void delete_channel();
        PN_CHAT_EXPORT void set_restrictions(Pubnub::String in_user_id, Pubnub::PubnubRestrictionsData restrictions);
        PN_CHAT_EXPORT void send_text(Pubnub::String message, pubnub_chat_message_type message_type, Pubnub::String meta_data);
        PN_CHAT_EXPORT std::vector<Pubnub::String> who_is_present();
        PN_CHAT_EXPORT bool is_present(Pubnub::String user_id);
        PN_CHAT_EXPORT std::vector<Pubnub::Message> get_history(Pubnub::String start_timetoken, Pubnub::String end_timetoken, int count);
        PN_CHAT_EXPORT Pubnub::Message get_message(Pubnub::String timetoken);
        PN_CHAT_EXPORT Pubnub::PubnubRestrictionsData get_user_restrictions(Pubnub::String in_user_id, Pubnub::String in_channel_id, int limit, String start, String end);
        PN_CHAT_EXPORT std::vector<Pubnub::Membership> get_members(int limit, Pubnub::String start_timetoken, Pubnub::String end_timetoken);

        PN_CHAT_EXPORT Pubnub::String get_channel_id();
        PN_CHAT_EXPORT ChatChannelData get_channel_data();


        ChatChannelData channel_data_from_json(Pubnub::String json_string);
        Pubnub::String channel_id_from_json(Pubnub::String json_string);
        Pubnub::String channel_data_to_json(Pubnub::String in_channel_id, ChatChannelData in_channel_data);

        private:
        Pubnub::Chat& chat_obj;
        Pubnub::String channel_id;
        ChatChannelData channel_data;

        Pubnub::String chat_message_to_publish_string(Pubnub::String message, pubnub_chat_message_type message_type);
    };
}
#endif /* CHANNEL_H */
