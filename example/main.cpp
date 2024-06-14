#include "c_functions/c_channel.hpp"
#include "c_functions/c_chat.hpp"
#include "chat.hpp"
#include "chat/message.hpp"
#include "chat/user.hpp"
#include "chat/channel.hpp"
#include "enums.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    
    std::string pub_key = "pub-c-79961364-c3e6-4e48-8d8d-fe4f34e228bf";
    std::string sub_key = "sub-c-2b4db8f2-c025-4a76-9e23-326123298667";
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
    Pubnub::Chat chat(pub_key.c_str(), sub_key.c_str(), user.c_str());
    Pubnub::ChatChannelData channel_data;
    channel_data.channel_name = "iksde2";
    channel_data.description = "Wha";
    Pubnub::Channel channel = chat.create_public_conversation("my_channel2", channel_data);

    auto message_callback = [](Pubnub::Message message){
        std::cout << "message received" << message.text() << std::endl;
    };

    channel.join(message_callback);

    Pubnub::ChatUserData user_data;
    user_data.user_name = "user name";
    Pubnub::User chat_user = chat.create_user(user.c_str(), user_data);

    channel.send_text("text from cpp", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::vector<Pubnub::String> present_users = channel.who_is_present();

    for(auto present_user : present_users)
    {
        std::cout << "present user: " << present_user << std::endl;
    }

    //channel.get_history();

    auto channel_callback = [](Pubnub::Channel channel){
        
        Pubnub::String message_on_update = "channel update. id: " + channel.get_channel_id(); 
        std::cout << message_on_update << std::endl;
    };
    channel.stream_updates(channel_callback);

    auto user_callback = [](Pubnub::User user){
        std::cout << "user update callback received. user name:  " << std::endl;
        //std::cout << "user update callback received. user name:  " << user.get_user_data().user_name << std::endl;
    };
    chat_user.stream_updates(user_callback);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::this_thread::sleep_for(std::chrono::seconds(3));
    channel_data.channel_name = "updated";
    channel.update(channel_data);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    user_data;
    user_data.user_name = "cpp user name";
    chat_user.update(user_data);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    channel.send_text("end of cpp", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "");
}
