#include "pubnub_chat/message.hpp"
#include "pubnub_chat/chat.hpp"
#include <chrono>
#include <thread>

int main() {
    auto subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
    auto publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
    auto user_id = std::getenv("USER_ID");
    
    // Create a chat instance
    auto chat = Pubnub::Chat::init(publish_key, subscribe_key, user_id, Pubnub::ChatConfig{});

    // Create channel that will publish message to forward
    auto channel = chat.create_public_conversation("base_channel", Pubnub::ChatChannelData{});

    // Create channel that will receive forwarded message
    auto forwarding_channel = chat.create_public_conversation("forwarding_channel", Pubnub::ChatChannelData{});

    // Listen for messages in the receiving channel
    forwarding_channel.connect([](const Pubnub::Message& message) {
            std::cout << message.text() << std::endl;
    });

    // Forward message from `base_channel` to `forwarding_channel`
    channel.connect([&chat, &forwarding_channel](const Pubnub::Message& message) {
            chat.forward_message(message, forwarding_channel);
    });

    // Send message
    channel.send_text("Chuje muje dzikie weze");

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "End of main" << std::endl;

    return 0;
}
