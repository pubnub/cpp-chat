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

    // Different deletion options for messages
    channel.connect([&chat](const Pubnub::Message& message) {
            std::cout << "Received message: " << message.text() << std::endl;

            auto deleted = message.delete_message();

            std::cout << "Deleted: " << deleted.text() << std::endl;
            std::cout << "Is `deleted` deleted: " << deleted.deleted() << std::endl;

            auto restored = deleted.restore();

            std::cout << "Restored: " << restored.text() << std::endl;
            std::cout << "Is `restored` deleted: " << restored.deleted() << std::endl;

            auto is_deleted_from_pubnub_infra = restored.delete_message_hard();

            std::cout << "Deleted from pubnub infra: " << is_deleted_from_pubnub_infra << std::endl;
    });

    // Send message
    channel.send_text("Message to delete for some reasons");

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "End of main" << std::endl;

    return 0;
}
