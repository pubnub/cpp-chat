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
    //Pubnub::Channel channel = chat.get_channel("my_channel2"); //use instead of line above to test get_channel

    //Create channel to delete
    Pubnub::ChatChannelData delete_channel_data;
    delete_channel_data.channel_name = "to_delete";
    Pubnub::Channel channel_to_delete = chat.create_public_conversation("to_delete", channel_data);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //STREAM PRESENCE
    auto presence_callback = [](std::vector<Pubnub::String> user_ids){ 
        std::cout << "stream presence ids: " << std::endl;
        for(auto user_id : user_ids)
        {
            std::cout << user_id << std::endl;
        }
        std::cout << std::endl;
    };
    channel.stream_presence(presence_callback);

    //JOIN AND STREAM MESSAGES
    auto message_callback = [](Pubnub::Message message){
        std::cout << "message received: " << message.text() << std::endl;
        std::cout << std::endl;
    };
    channel.join(message_callback);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //DELETE CHANNEL
    //channel_to_delete.delete_channel();
    chat.delete_channel(channel_to_delete.get_channel_id());

    //GET CHANNELS TEST
    std::cout << "get channels, you shouldn't see channel to delete here!!" << std::endl;
    std::vector<Pubnub::Channel> channels = chat.get_channels("", 30, "1708181488", "1728181488");
    for(auto received_channel : channels)
    {
        std::cout << "get channels, channel: " << received_channel.get_channel_id() << std::endl;
    }
    std::cout << std::endl;

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
    std::cout << std::endl;

    //LISTEN FOR EVENTS
    auto event_callback = [](Pubnub::String event){ 
        std::cout << "event callback: " <<  event << std::endl;
        std::cout << std::endl;
    };
    chat.listen_for_events(channel.get_channel_id(), event_callback);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //SET RESTRICTIONS FROM CHANNEL
    Pubnub::PubnubRestrictionsData restrictions_data;
    restrictions_data.mute = true;
    restrictions_data.reason = "bo tak";
    channel.set_restrictions(user, restrictions_data);



    /* USERS */

    //CREATE USER
    Pubnub::ChatUserData user_data;
    user_data.user_name = "chat user name";
    Pubnub::User chat_user = chat.create_user(user.c_str(), user_data);
    //Pubnub::User chat_user = chat.get_user(user.c_str()); //use instead of line above to test get_user

    //Create user to delete
    Pubnub::ChatUserData delete_user_data;
    delete_user_data.user_name = "user to delete";
    Pubnub::User user_to_delete = chat.create_user("to_delete", delete_user_data);

    //STREAM USER UPDATES
    auto user_callback = [](Pubnub::User user){
        std::cout << "user update callback. User name:  " << user.get_user_data().user_name << std::endl;
        std::cout << std::endl;
    };
    chat_user.stream_updates(user_callback);

    //DELETE USER
    //user_to_delete.delete_user();
    chat.delete_user(user_to_delete.get_user_id());

    //GET_USERS
    std::cout << "get users, you shouldn't see user to delete here!!" << std::endl;
    std::vector<Pubnub::User> users = chat.get_users("", 10, "1738181488", "1718361914");
    for(auto received_user : users)
    {
        std::cout << "get users, user: " << received_user.get_user_id() << std::endl;
    }
    std::cout << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    /* PRESENCE */

    //WHERE PRESENT
    std::cout << "PRESENCE CHECK"  << std::endl;
    std::vector<Pubnub::String> present_channels = chat_user.where_present();
    for(auto present_channel : present_channels)
    {
        std::cout << "user is present in: " << present_channel << std::endl;
    }
    std::cout << std::endl;

    //IS PRESENT ON
    bool is_user_present = chat_user.is_present_on(channel.get_channel_id());
    std::cout << "is present on (should be true): " << is_user_present << std::endl;
    
    //IS PRESENT
    bool is_user_present_in_chnnel = channel.is_present(chat_user.get_user_id());
    std::cout << "is present (should be true): " << is_user_present_in_chnnel << std::endl;


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

    //update user from user
    user_data.user_name = "updated from user";
    chat_user.update(user_data);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //update user from chat
    user_data.user_name = "user updated from chat";
    chat.update_user(chat_user.get_user_id(),  user_data);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //leave channel
    channel.leave();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //send message after leaving channel. This shouldn't be received
    channel.send_text("shouldn't see this message", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "");


    std::this_thread::sleep_for(std::chrono::seconds(1));

    //report user, it should emit an event

    //chat_user.report("a o tak o sobie");
    std::this_thread::sleep_for(std::chrono::seconds(1));


    std::cout << "end of main" << std::endl;
    


}
