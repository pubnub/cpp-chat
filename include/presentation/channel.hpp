#ifndef PN_CHAT_CHANNEL_HPP
#define PN_CHAT_CHANNEL_HPP

#include "string.hpp"
#include "export.hpp"
#include "restrictions.hpp"
#include "enums.hpp"
#include <memory>
#include <vector>
#include <functional>

class ChannelService; 

namespace Pubnub 
{
    class Message;

    struct ChatChannelData
    {
        Pubnub::String channel_name = "";
        Pubnub::String description = "";
        Pubnub::String custom_data_json = "";
        Pubnub::String updated = "";
        Pubnub::String status = "";
        Pubnub::String type = "";
    };

    class Channel {
        public:
            PN_CHAT_EXPORT inline Pubnub::String channel_id(){return channel_id_internal;};
            PN_CHAT_EXPORT Pubnub::ChatChannelData channel_data();

            PN_CHAT_EXPORT Pubnub::Channel update(ChatChannelData in_additional_channel_data);
            PN_CHAT_EXPORT void connect(std::function<void(Message)> message_callback);
            PN_CHAT_EXPORT void disconnect();
            PN_CHAT_EXPORT void join(std::function<void(Message)> message_callback, Pubnub::String additional_params = "");
            PN_CHAT_EXPORT void leave();
            PN_CHAT_EXPORT void delete_channel();
            PN_CHAT_EXPORT void send_text(Pubnub::String message, pubnub_chat_message_type message_type, Pubnub::String meta_data);
            PN_CHAT_EXPORT std::vector<Pubnub::String> who_is_present();
            PN_CHAT_EXPORT bool is_present(Pubnub::String user_id);

        private:
            PN_CHAT_EXPORT Channel(std::shared_ptr<ChannelService> channel_service, Pubnub::String channel_id);
            std::shared_ptr<ChannelService> channel_service;
            Pubnub::String channel_id_internal;

        friend class ChannelService;
    };
};

#endif // PN_CHAT_CHANNEL_HPP