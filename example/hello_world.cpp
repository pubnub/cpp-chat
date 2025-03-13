#include <cstdlib>
#include <thread>
#include "chat.hpp"

int main() {
    auto subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
    auto publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
    auto user_id = std::getenv("USER_ID");
    
    // Create a chat instance
    auto chat = Pubnub::Chat::init(publish_key, subscribe_key, user_id, Pubnub::ChatConfig{});

    // Create basic public conversation
    auto channel = chat.create_public_conversation("base_channel", Pubnub::ChatChannelData{});

    channel.connect([](Pubnub::Message message) {
        std::cout << "Received message: " << message.text() << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel.send_text("Hello, world!");

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "End of main" << std::endl;

    return 0;
}
