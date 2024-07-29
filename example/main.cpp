#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/channel.hpp"
#include "pubnub_chat/restrictions.hpp"
#include "pubnub_chat/user.hpp"

#include "enums.hpp"
#include <chrono>
#include <string>
#include <iostream>
#include <thread>

int main() {
    std::string pub_key = "pub-c-79961364-c3e6-4e48-8d8d-fe4f34e228bf";
    std::string sub_key = "sub-c-2b4db8f2-c025-4a76-9e23-326123298667";
    std::string user = "hehehe";

    Pubnub::ChatConfig config;
    config.publish_key = pub_key;
    config.subscribe_key = sub_key;
    config.user_id = user;

    Pubnub::Chat chat(config);

    // auto vecint = chat.TestVector();

    // for(auto &elem : vecint)
    // {
    //     std::cout << "vector: " << elem << std::endl;
    // }

         //CREATE CHANNEL  
    Pubnub::ChatChannelData channel_data;
    channel_data.channel_name = "iksde2";
    channel_data.description = "Wha";
    //Pubnub::Channel channel = chat.create_public_conversation("my_test_channel2", channel_data);

    Pubnub::Channel channel = chat.get_channel("my_test_channel3");

    auto response = chat.mark_all_messages_as_read("", "", 3);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto memberships = response.memberships.into_std_vector();

    for(auto membership : memberships)
    {
        std::cout << "membrship. Channel ID: " << membership.channel.channel_id() << " Custom data: " << membership.custom_data() << std::endl;
        std::cout << "Channel name: " << membership.channel.channel_data().channel_name << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto response2 = chat.get_unread_messages_counts("", "", "", 3);
    auto cpp_response2 = response2.into_std_vector();

    for(auto res : cpp_response2)
    {
        std::cout << "membrship. Channel ID: " << res.channel.channel_id() << " Custom data: " << res.membership.custom_data() << std::endl;
        std::cout << "Message count: " << res.count << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

//     auto messages = channel.get_history((char*)NULL, (char*)NULL, 10);

//     for(auto message : messages)
//     {
//         std::cout << "message: " << message.message_data().text << std::endl;
//     }

//     std::cout << std::endl;

//     auto get_message = channel.get_message("17218251435862406");

//     std::cout << "get message: " << get_message.message_data().text << std::endl;


//     auto channels = chat.get_channels("", 10, "1738181488", "1708181488");

//     //Pubnub::Vector<Pubnub::Channel> new_channels;
// //
//     //for(auto &channel : channels)
//     //{
//     //    new_channels.push_back(channel);
//     //}
    
//     for(auto new_channel : channels)
//     {
//         std::cout << "new channel: " << new_channel.channel_id() << std::endl;
//     }

    // for(int i = 0; i < vecint.size(); i++)
    // {
    //     std::cout << "vector: " << vecint.get_element(i) << std::endl;
    // }


    std::this_thread::sleep_for(std::chrono::seconds(1));



//    Pubnub::ChatUserData user_data;
//    user_data.user_name = "chat user name";
//    auto user_obj = chat.create_user(user.c_str(), user_data);
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    auto user_to_invite = chat.create_user("invitee", Pubnub::ChatUserData());
//
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//
//
//    Pubnub::CreatedChannelWrapper created_channel = chat.create_direct_conversation(user_to_invite, "test_private_channel", Pubnub::ChatChannelData());
//
//    created_channel.created_channel.join([](Pubnub::Message message)
//    {
//        std::cout << "message from private channel: " << message.text() << std::endl;
//            
//    });
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//
//    created_channel.created_channel.get_typing([=](std::vector<Pubnub::String> users_ids){
//        std::cout << "typing event"<< std::endl;
//        for(auto id : users_ids)
//        {
//            std::cout << "typing user: " << id << std::endl;
//        }
//        std::cout << std::endl;
//    });
//        std::this_thread::sleep_for(std::chrono::seconds(2));
//
//
//
//    std::cout << "start typing call" << std::endl;
//    created_channel.created_channel.start_typing();
//
//    created_channel.created_channel.send_text("random message");
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//
//    std::cout << "stop typing call" << std::endl;
//    created_channel.created_channel.stop_typing();
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//
//    created_channel.created_channel.send_text("message to private channel", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "");
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//
//    std::cout << "start typing call 2" << std::endl;
//    created_channel.created_channel.start_typing();
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//
//    std::this_thread::sleep_for(std::chrono::seconds(3));
//
//
//    std::this_thread::sleep_for(std::chrono::seconds(3));
//    created_channel.created_channel.start_typing();
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//
//
//
//      /* CHANNELS */

//      //CREATE CHANNEL  
//      Pubnub::ChatChannelData channel_data;
//      channel_data.channel_name = "iksde2";
//      channel_data.description = "Wha";
//      Pubnub::Channel channel = chat.create_public_conversation("my_test_channel2", channel_data);
//      //Pubnub::Channel channel = chat.get_channel("my_channel2"); //use instead of line above to test get_channel

//      //Create channel to delete
//      Pubnub::ChatChannelData delete_channel_data;
//      delete_channel_data.channel_name = "to_delete";
//      Pubnub::Channel channel_to_delete = chat.create_public_conversation("to_delete", channel_data);

//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      //STREAM PRESENCE
//      auto presence_callback = [](std::vector<Pubnub::String> user_ids){ 
//          std::cout << "stream presence ids: " << std::endl;
//          for(auto user_id : user_ids)
//          {
//              std::cout << user_id << std::endl;
//          }
//          std::cout << std::endl;
//      };
//      channel.stream_presence(presence_callback);

//      //JOIN AND STREAM MESSAGES
//      auto message_callback = [](Pubnub::Message message){
//          std::cout << "message received: " << message.text() << std::endl;
//          std::cout << std::endl;
//      };
//      channel.join(message_callback);

//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      //DELETE CHANNEL
//      //channel_to_delete.delete_channel();
//      chat.delete_channel(channel_to_delete.channel_id());

//      //GET CHANNELS TEST
//      std::cout << "get channels, you shouldn't see channel to delete here!!" << std::endl;
//      std::vector<Pubnub::Channel> channels = chat.get_channels("", 30, "1728181488", "1708181488");
//      for(auto received_channel : channels)
//      {
//          std::cout << "get channels, channel: " << received_channel.channel_id() << std::endl;
//      }
//      std::cout << std::endl;

//      //STREAM CHANNEL UPDATES
//      auto channel_callback = [](Pubnub::Channel channel){ 
//          Pubnub::String message_on_update = "channel update callback. channel name: " + channel.channel_data().channel_name; 
//          std::cout << message_on_update << std::endl;
//      };
//      channel.stream_updates(channel_callback);

//      //CHANNEL GET MEMBERS
//      std::vector<Pubnub::Membership> memberships = channel.get_members(3, "17386132136530924", "17086132136530924");
//          for(auto membership : memberships)
//      {
//          std::cout << "get memberships. membership user:  " << membership.user.user_id() <<  " channel: " << membership.channel.channel_id() << std::endl;
//      }
//      std::cout << std::endl;

//      //LISTEN FOR EVENTS
//      auto event_callback = [](Pubnub::String event){ 
//          std::cout << "event callback: " <<  event << std::endl;
//          std::cout << std::endl;
//      };
//      // TODO: something is wrong here
//  //    chat.listen_for_events(channel.get_channel_id(), event_callback);

//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      //SET RESTRICTIONS FROM CHANNEL
//      Pubnub::Restriction restrictions_data;
//      restrictions_data.mute = true;
//      restrictions_data.reason = "bo tak";
//      channel.set_restrictions(user, restrictions_data);

//      //GET CHANNEL HISTORY
//      std::cout <<  std::endl;
//      std::cout <<"GET HISTORY: " <<  std::endl;
//      std::vector<Pubnub::Message> history_messages = channel.get_history("99999999999999999", "00000000000000000", 5);
//      Pubnub::String message_token;
//      for(auto history_message : history_messages)
//      {
//          std::cout <<"history message: " <<  history_message.message_data().text << std::endl;
//          message_token = history_message.timetoken();
//      }



//      /* MESSAGES */


//      //GET MESSAGE
//      std::cout <<  std::endl;

//      // TODO: this seems to not work
//  //    Pubnub::String message_token = "17207041559619450";
//      Pubnub::Message message_from_get = channel.get_message(message_token);
//      std::cout <<"Message from get: " <<  message_from_get.text() << std::endl;
//      std::cout <<  std::endl;

//      //STREAM MESSAGE UPDATES
//      auto message_update_callback = [](Pubnub::Message message){
//          std::cout << "message update received: " << message.text() << std::endl;
//          std::cout << std::endl;
//      };
//      message_from_get.stream_updates(message_update_callback);

//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      //EDIT MESSAGE
//      //message_from_get.edit_text("newly edited"); //change message for each test or server will throw an exception

//      /* DELETE MESSAGE TEST
//      std::cout <<"Is message deleted: " <<  message_from_get.deleted() << std::endl;
//      message_from_get.delete_message();
//      std::this_thread::sleep_for(std::chrono::seconds(1));
//      std::cout <<"Is message deleted: " <<  message_from_get.deleted() << std::endl;
//      */



//      /* USERS */


//      //CREATE USER
//      Pubnub::ChatUserData user_data;
//      user_data.user_name = "chat user name";
//      Pubnub::User chat_user = chat.create_user(user.c_str(), user_data);
//      //Pubnub::User chat_user = chat.get_user(user.c_str()); //use instead of line above to test get_user

//      //Create user to delete
//      Pubnub::ChatUserData delete_user_data;
//      delete_user_data.user_name = "user to delete";
//      Pubnub::User user_to_delete = chat.create_user("to_delete", delete_user_data);

//      //STREAM USER UPDATES
//      auto user_callback = [](Pubnub::User user){
//          std::cout << "user update callback. User name:  " << user.user_data().user_name << std::endl;
//          std::cout << std::endl;
//      };
//      chat_user.stream_updates(user_callback);

//      //DELETE USER
//      //user_to_delete.delete_user();
//      chat.delete_user(user_to_delete.user_id());

//      //GET_USERS
//      std::cout << "get users, you shouldn't see user to delete here!!" << std::endl;
//      std::vector<Pubnub::User> users = chat.get_users("", 10, "1738181488", "1718361914");
//      for(auto received_user : users)
//      {
//          std::cout << "get users, user: " << received_user.user_id() << std::endl;
//      }
//      std::cout << std::endl;

//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      /* PRESENCE */

//      //WHERE PRESENT
//      std::cout << "PRESENCE CHECK"  << std::endl;
//      std::vector<Pubnub::String> present_channels = chat_user.where_present();
//      for(auto present_channel : present_channels)
//      {
//          std::cout << "user is present in: " << present_channel << std::endl;
//      }
//      std::cout << std::endl;

//      //IS PRESENT ON
//      bool is_user_present = chat_user.is_present_on(channel.channel_id());
//      std::cout << chat_user.user_id() << " is present on "<< channel.channel_id() <<" (should be true): " << is_user_present << std::endl;
    
//      //IS PRESENT
//      bool is_user_present_in_chnnel = channel.is_present(chat_user.user_id());
//      std::cout << chat_user.user_id() << " is present on " << channel.channel_id() << " (should be true): " << is_user_present_in_chnnel << std::endl;

//      //REPORT USER
//      chat_user.report("a o tak o sobie");


//      /* EXECUTION FLOW */
//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      //update channel from channel
//      channel_data.channel_name = "updated from channel";
//      channel.update(channel_data);

//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      //update channel from chat
//      channel_data.channel_name = "updated from chat";
//      chat.update_channel(channel.channel_id(), channel_data);

//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      //send message
//      channel.send_text("text from main", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "");

//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      //update user from user
//      user_data.user_name = "updated from user";
//      chat_user.update(user_data);

//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      //update user from chat
//      user_data.user_name = "user updated from chat";
//      chat.update_user(chat_user.user_id(),  user_data);


//      std::this_thread::sleep_for(std::chrono::seconds(1));
//      //leave channel
//      channel.leave();
//      std::this_thread::sleep_for(std::chrono::seconds(1));

//      //send message after leaving channel. This shouldn't be received
//      channel.send_text("shouldn't see this message", Pubnub::pubnub_chat_message_type::PCMT_TEXT, "");


//      std::this_thread::sleep_for(std::chrono::seconds(1));

     //report user, it should emit an event




    std::cout << "end of main" << std::endl;
    

}
