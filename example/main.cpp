#include "c_functions/c_channel.hpp"
#include "c_functions/c_chat.hpp"
#include "chat.hpp"
#include "chat/message.hpp"
#include "chat/membership.hpp"
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

    Pubnub::Chat chat(pub_key.c_str(), sub_key.c_str(), user.c_str());



    /* CHANNELS */

    //CREATE CHANNEL  
    Pubnub::ChatChannelData channel_data;
    channel_data.channel_name = "iksde2";
    channel_data.description = "Wha";
    Pubnub::Channel channel = chat.create_public_conversation("my_channel2", channel_data);

    //Create channel to delete
    Pubnub::ChatChannelData delete_channel_data;
    delete_channel_data.channel_name = "to_delete";
    Pubnub::Channel channel_to_delete = chat.create_public_conversation("to_delete", channel_data);

    //JOIN AND STREAM MESSAGES
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto message_callback = [](Pubnub::Message message){
        std::cout << "message received: " << message.text() << std::endl;
    };
    channel.join(message_callback);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //DELETE CHANNEL
    channel_to_delete.delete_channel();

    //GET CHANNELS TEST
    std::cout << "get channels, you shouldn't see channel to delete here!!" << std::endl;
    std::vector<Pubnub::Channel> channels = chat.get_channels("", 3, "1708181488", "1728181488");
    for(auto received_channel : channels)
    {
        std::cout << "get channels, channel: " << received_channel.get_channel_id() << std::endl;
    }

    //STREAM CHANNEL UPDATES
    auto channel_callback = [](Pubnub::Channel channel){ 
        Pubnub::String message_on_update = "channel update callback. channel name: " + channel.get_channel_data().channel_name; 
        std::cout << message_on_update << std::endl;
    };
    channel.stream_updates(channel_callback);

    //CHANNEL GET MEMBERS
    std::vector<Pubnub::Membership> memberships = channel.get_members(3, "1708181488", "1728181488");
        for(auto membership : memberships)
    {
        std::cout << "get memberships. membership user:  " << membership.get_user_id() <<  " channel: " << membership.get_channel_id() << std::endl;
    }




    /* EXECUTION FLOW */
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //update channel from channel
    channel_data.channel_name = "updated from channel";
    channel.update(channel_data);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //update channel from chat
    channel_data.channel_name = "updated from chat";
    chat.update_channel(channel.get_channel_id(), channel_data);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //send message
    channel.send_text("text from main", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "");

    std::this_thread::sleep_for(std::chrono::seconds(1));

    channel.leave();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    channel.send_text("shouldn't see this message", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));


    std::cout << "end of main" << std::endl;
    


}
