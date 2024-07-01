#include "string.hpp"
#include <memory>

class ChannelService; 

namespace Pubnub {
    struct ChannelData
    {
        Pubnub::String channel_name;
        Pubnub::String description;
        Pubnub::String custom_data_json;
        Pubnub::String updated;
        Pubnub::String status;
        Pubnub::String type;
    };

    PN_CHAT_EXPORT class Channel {
        private:
            PN_CHAT_EXPORT Channel(String channel_id, std::shared_ptr<ChannelService> channel_service);
            String channel_id;
            std::shared_ptr<ChannelService> channel_service;


        friend class ChannelService;
    };
};
