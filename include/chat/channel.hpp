#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include <vector>
#include <functional>
#include <map>
#include "export.hpp"
#include "enums.hpp"
#include "string.hpp"
#include "callbacks.hpp"
#include "infra/timer.hpp"

extern "C" {
    #include "core/pubnub_api_types.h"
}

namespace Pubnub
{
    class Chat;
    class Message;
    class Membership;
    class User;
    struct PubnubRestrictionsData;

    struct ChatChannelData
    {
        Pubnub::String channel_name;
        Pubnub::String description;
        Pubnub::String custom_data_json;
        Pubnub::String updated;
        Pubnub::String status;
        Pubnub::String type;
    };


    class Channel
    {
        public:

        PN_CHAT_EXPORT Channel(Pubnub::Chat& InChat, Pubnub::String in_channel_id, ChatChannelData in_additional_channel_data);
        PN_CHAT_EXPORT Channel(Pubnub::Chat& InChat, Pubnub::String in_channel_id, Pubnub::String channel_data_json);
        //From user membership Json
        PN_CHAT_EXPORT Channel(Pubnub::Chat& InChat, Pubnub::String channel_data_json);
        ~Channel() = default;

        PN_CHAT_EXPORT Pubnub::Channel update(ChatChannelData in_additional_channel_data);
        PN_CHAT_EXPORT void connect(std::function<void(Message)> message_callback);
        PN_CHAT_EXPORT void connect(CallbackStringFunction string_callback);
        PN_CHAT_EXPORT std::vector<Pubnub::String> connect_and_get_messages();
        PN_CHAT_EXPORT void disconnect();
        PN_CHAT_EXPORT void join(std::function<void(Message)> message_callback, Pubnub::String additional_params = "");
        PN_CHAT_EXPORT void join(CallbackStringFunction string_callback, Pubnub::String additional_params = "");
        PN_CHAT_EXPORT std::vector<String> join_and_get_messages(Pubnub::String additional_params = "");
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
        PN_CHAT_EXPORT Pubnub::Membership invite(Pubnub::User user);
        PN_CHAT_EXPORT std::vector<Pubnub::Membership> invite_multiple(std::vector<Pubnub::User> users);

        PN_CHAT_EXPORT void start_typing();
        PN_CHAT_EXPORT void stop_typing();
        PN_CHAT_EXPORT void get_typing(std::function<void(std::vector<Pubnub::String>)> typing_callback);

        PN_CHAT_EXPORT Pubnub::Channel& pin_message(Pubnub::Message &message);
        PN_CHAT_EXPORT Pubnub::Channel& unpin_message();
        PN_CHAT_EXPORT Pubnub::Message get_pinned_message();

        PN_CHAT_EXPORT void stream_updates(std::function<void(Channel)> channel_callback);
        PN_CHAT_EXPORT void stream_updates_on(std::vector<Pubnub::Channel> channels, std::function<void(Channel)> channel_callback);
        PN_CHAT_EXPORT void stream_presence(std::function<void(std::vector<Pubnub::String>)> presence_callback);

        PN_CHAT_EXPORT Pubnub::String get_channel_id();
        PN_CHAT_EXPORT ChatChannelData get_channel_data();


        ChatChannelData channel_data_from_json(Pubnub::String json_string);
        Pubnub::String channel_id_from_json(Pubnub::String json_string);
        Pubnub::String channel_data_to_json(Pubnub::String in_channel_id, ChatChannelData in_channel_data);

        private:
        Pubnub::Chat& chat_obj;
        Pubnub::String channel_id;
        ChatChannelData channel_data;

        //TYPING
        //TODO: Move this to config
        int TYPING_TIMEOUT = 5000;

        bool typing_sent = false;
        Timer typing_sent_timer;
        std::map<Pubnub::String, Timer, Pubnub::StringComparer> typing_indicators;



        Pubnub::String chat_message_to_publish_string(Pubnub::String message, pubnub_chat_message_type message_type);
    };
}
#endif /* CHANNEL_H */
