#include "c_functions/c_channel.hpp"
#include "c_functions/c_chat.hpp"
#include "chat.hpp"
#include "chat/message.hpp"
#include "enums.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    std::string pub_key = "demo";
    std::string sub_key = "demo";
    std::string user = "hehehe";

//    Pubnub::Chat chat(pub_key, sub_key, user);
//
//    Pubnub::ChatChannelData channel_data;
//    channel_data.channel_name = "iksde2";
//    channel_data.description = "Wha";
//    channel_data.status = "1";
//    channel_data.type = "hm";
//    channel_data.updated = "true";
//    channel_data.custom_data_json = "{}";
//
//    Pubnub::Channel channel = chat.create_public_conversation("iksdeedski", channel_data );
//    //channel.connect([](Pubnub::Message message) {
//    //    std::cout << message.get_message_data().text << std::endl;
//    //});
//    channel.connect_and_get_messages();
//    
//    channel.send_text("ARE YOU HEARING ME YOU BASTARDS?", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "");
//
//    std::this_thread::sleep_for(std::chrono::seconds(30));
    auto chat = pn_chat_new(pub_key.c_str(), sub_key.c_str(), user.c_str());
    chat->get_pubnub_context().stop_resolving_callbacks();
    auto channel = pn_chat_create_public_conversation_dirty(chat, "test", "test", "fuck", "{}", "true", "1", "sure");

    auto mes1 = pn_channel_connect(channel);
    auto kekw = true;

    auto watek = std::thread([&kekw, &chat] {
        while (kekw) {
            auto mes = pn_chat_get_messages(chat, "chuj");

            std::cout << mes << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    std::cout << mes1 << std::endl;

    pn_channel_send_text(channel, "xDDDD", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "{}");

    std::this_thread::sleep_for(std::chrono::seconds(1));

    pn_channel_send_text(channel, "xDDDD2", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "{}");

    std::this_thread::sleep_for(std::chrono::seconds(1));

    pn_channel_send_text(channel, "xDDDD3", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "{}");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    pn_channel_send_text(channel, "xDDDD4", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "{}");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    kekw = false;

    watek.join();
}
