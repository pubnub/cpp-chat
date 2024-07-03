#ifndef PN_CHAT_CHANNEL_HPP
#define PN_CHAT_CHANNEL_HPP

#include "string.hpp"
#include "export.hpp"
#include <memory>

class ChannelService; 

namespace Pubnub 
{
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

        private:
            PN_CHAT_EXPORT Channel(std::shared_ptr<ChannelService> channel_service, Pubnub::String channel_id);
            std::shared_ptr<ChannelService> channel_service;
            Pubnub::String channel_id_internal;

        friend class ChannelService;
    };
};

#endif // PN_CHAT_CHANNEL_HPP