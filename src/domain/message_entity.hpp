#include "enums.hpp"
#include "string.hpp"

class MessageEntity {
    public:
        struct MessageActionData
        {
            Pubnub::pubnub_message_action_type type;
            Pubnub::String value;
            Pubnub::String timetoken;
            Pubnub::String user_id;
        };

        struct MessageData
        {
            Pubnub::pubnub_chat_message_type type;
            Pubnub::String text;
            Pubnub::String channel_id;
            //user_id doesn't work for now, because we don't get this info from pubnub_fetch_history
            Pubnub::String user_id;
            //meta doesn't work for now, because we don't get this info from pubnub_fetch_history
            Pubnub::String meta;
            std::vector<MessageActionData> message_actions;
        };

        MessageEntity(Pubnub::String timetoken, MessageData message_data);

    private:
        Pubnub::String timetoken;
        MessageData message_data;
};
