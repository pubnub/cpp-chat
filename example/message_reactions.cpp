#include "const_values.hpp"
#include "enums.hpp"
#include "pubnub_chat/message.hpp"
#include "pubnub_chat/chat.hpp"
#include "string.hpp"
#include <chrono>
#include <thread>

int main() {
    auto subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
    auto publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
    auto user_id = std::getenv("USER_ID");
    
    // Create a chat instance
    auto chat = Pubnub::Chat::init(publish_key, subscribe_key, user_id, Pubnub::ChatConfig{});

    // Create channel that will publish message that will be the thread initializer
    auto channel = chat.create_public_conversation("base_channel", Pubnub::ChatChannelData{});

    // In your application you should keep handle to the message to be able to manipulate it later
    // or you can always get the message via timetoken got from `channel.connect() => message.timetoken()`
    Pubnub::Message message_to_react;

    channel.connect([&channel, &message_to_react](Pubnub::Message message) {
        std::cout << "Received message: " << message.text() << std::endl << std::endl;

        message_to_react = message.toggle_reaction("cool!");
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));


    channel.send_text("Some cool message that people will react!");

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // This reactions are from the local state of the application
    std::cout << "Reactions from local state:" << std::endl;
    std::cout << "Check if locally there are `cool!` reactions: " << message_to_react.has_user_reaction("cool!") << std::endl; 
    for (auto reactions : message_to_react.reactions()) {
        std::cout << "Reaction: `" << reactions.value << "` by user: " << reactions.user_id << std::endl;
    }

    std::cout << std::endl;

    // This reactions are from the server
    auto message = channel.get_message(message_to_react.timetoken());
    std::cout << "Reactions from server:" << std::endl;
    std::cout << "Check if there are `cool!` reactions: " << message.has_user_reaction("cool!") << std::endl;
    for (auto reactions : message.reactions()) {
        std::cout << "Reaction: `" << reactions.value << "` by user: " << reactions.user_id << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "End of main" << std::endl;
}

