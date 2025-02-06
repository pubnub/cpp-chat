#include <cstdlib>
#include <thread>
#include "chat.hpp"

// Warning: This example does not take into the account synchronization of the threads.
// In your code you should use some synchronization mechanism to avoid data
// races between callbacks and main thread. For example, you can use `std::mutex`.

int main() {
    auto subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
    auto publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
    auto user_id = std::getenv("USER_ID");
    
    // Create a chat instance
    auto chat = Pubnub::Chat::init(publish_key, subscribe_key, user_id, Pubnub::ChatConfig{});

    // Create basic public conversation
    auto channel = chat.create_public_conversation("base_channel", Pubnub::ChatChannelData{});

    // You can use any of our entities to call the `stream_updates` method
    channel.stream_updates([&channel](const Pubnub::Channel& updated_channel) {
        std::cout << "Update!" << std::endl;
        channel = updated_channel;
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Channel description before update [assuming empty]:" << channel.channel_data().description << std::endl;

    Pubnub::ChatChannelData new_data = channel.channel_data();
    new_data.description = "New description";

    channel.update(new_data);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Channel description after update: " << channel.channel_data().description << std::endl;

    std::cout << "End of main" << std::endl;

    return 0;
}
