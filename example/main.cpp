#include "chat.hpp"
#include "chat/message.hpp"

int main() {
    std::string pub_key = "pub-c-2451c2cf-9f04-446b-8f85-e06564095e55";
    std::string sub_key = "sub-c-d16ff59f-b415-4ef9-8c29-ddda64fa2b43";
    std::string user = "hehehe";

    Pubnub::Chat chat(pub_key, sub_key, user);

    Pubnub::ChatChannelData channel_data;
    channel_data.channel_name = "iksde2";
    channel_data.description = "Wha";
    channel_data.status = 1;
    channel_data.type = "hm";
    channel_data.updated = "true";
    channel_data.custom_data_json = "{}";

    Pubnub::Channel channel = chat.create_public_conversation("iksdeedski", channel_data );
    channel.join("");
    channel.send_text("ARE YOU HEARING ME YOU BASTARDS?", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "");
}

