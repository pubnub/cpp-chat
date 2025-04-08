#include <cstdlib>
#include <thread>

#include "chat.hpp"

int main() {
    try {
        auto subscribe_key = "sub-c-eaa73d83-9ed7-4e49-8b5a-1b1d64160735";
        auto publish_key = "pub-c-b23194b7-f485-43dc-9544-115997a7d9a9";
        auto user_id = "just_me";

        std::cout << "Starting chat..." << std::endl;

        // Create a chat instance
        auto chat = Pubnub::Chat::init(publish_key, subscribe_key, user_id, Pubnub::ChatConfig {});

        // Create basic public conversation
        auto channel = chat.get_channel("base_channel");

            channel.connect([](Pubnub::Message message) {
                std::cout << "Received message: " << message.text() << std::endl;
            });

        std::this_thread::sleep_for(std::chrono::seconds(3));

        channel.send_text("Hello, world!");

        std::this_thread::sleep_for(std::chrono::seconds(3));

        std::cout << "End of main" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}