#include <cstdlib>
#include <thread>
#include "c_channel.hpp"
#include "c_response.hpp"
#include "chat.hpp"

// This function makes sure that the user we want to type to exists
Pubnub::User prepare_user(Pubnub::Chat& chat);

int main() {
    auto subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
    auto publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
    auto user_id = std::getenv("USER_ID");

    // Create a chat instance
    auto chat = Pubnub::Chat::init(publish_key, subscribe_key, user_id, Pubnub::ChatConfig{});

    auto user_to_type_to = prepare_user(chat);

    // Create basic public conversation
    auto channel = chat.create_direct_conversation(user_to_type_to, "base_channel", Pubnub::ChatChannelData{}).created_channel;

    channel.connect([](Pubnub::Message message) {
        std::cout << "Received message: " << message.text() << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));

//    channel.get_typing([](Pubnub::Vector<Pubnub::String> users) {
//        std::cout << "Users typing: ";
//        for (auto user : users) {
//            std::cout << user << " ";
//        }
//        std::cout << std::endl;
//    });

    pn_channel_get_typing(&channel);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Assuming user started typing a message at this point
    // and it takes 3 seconds to type 
    std::cout << "User's starting typing" << std::endl;
    channel.start_typing();

    std::this_thread::sleep_for(std::chrono::seconds(3));
    char buffer[4096];
    pn_c_consume_response_buffer(&chat, buffer);
    std::cout << buffer << std::endl;



    std::cout << "User's sending message" << std::endl;
    channel.send_text("Hello, world!");
    std::cout << "User's stopping typing" << std::endl;
    channel.stop_typing();

    char buffer2[4096];
    pn_c_consume_response_buffer(&chat, buffer2);
    std::cout << buffer2 << std::endl;



    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "End of main" << std::endl;

    return 0;
}

Pubnub::User prepare_user(Pubnub::Chat& chat) {
    try {
        return chat.create_user("user_to_type_to", Pubnub::ChatUserData{});

    } catch (const std::exception& e) {
        // Assuming user already exists
        return chat.get_user("user_to_type_to");
    }
}
