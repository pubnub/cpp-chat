#include <cstdlib>
#include <thread>

#include "chat.hpp"
#include "map.hpp"
#include "string.hpp"
#include "vector.hpp"

int main() {
    auto subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
    auto publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
    auto sender_id = "sender_id";
    auto receiver_id = "receiver_id";

    // Create a chat instance
    auto chat_receiver =
        Pubnub::Chat::init(publish_key, subscribe_key, sender_id, Pubnub::ChatConfig {});
    auto chat_sender =
        Pubnub::Chat::init(publish_key, subscribe_key, receiver_id, Pubnub::ChatConfig {});

    auto sender = chat_sender.current_user();
    auto receiver = chat_receiver.current_user();

    // Create direct conversation
    auto conversation = chat_receiver.create_direct_conversation(sender, "chat_with_sender", Pubnub::ChatChannelData{});
    auto channel = conversation.created_channel;
    auto membership = conversation.host_membership;

    // TODO: This API sounds like a code smell
    channel.stream_read_receipts(
        [](Pubnub::Map<Pubnub::String, Pubnub::Vector<Pubnub::String>, Pubnub::StringComparer> reads
        ) {
            for (auto read : reads.into_std_map()) {
                std::cout << "User: " << read.first << " has read messages: ";
                for (auto message : read.second) {
                    std::cout << message << " ";
                }
                std::cout << std::endl;
            }
        }
    );
 
    channel.connect([membership](Pubnub::Message message) {
        std::cout << "Received message: " << message.text() << std::endl;
        // In your application you can now mark it as read
        membership.set_last_read_message(message);
    });
 
    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto sender_channel = chat_sender.get_channel("chat_with_sender");
    sender_channel.send_text("Hello Receiver!");
 
    std::this_thread::sleep_for(std::chrono::seconds(3));
 
    auto receiver_unread = chat_receiver.get_unread_messages_counts();
    auto sender_unread = chat_sender.get_unread_messages_counts();
 
    std::this_thread::sleep_for(std::chrono::seconds(3));
 
    std::cout << "Unread messages for receiver: " << receiver_unread.size() << std::endl;
    for (auto unread : receiver_unread) {
        std::cout << "Channel: " << unread.channel.channel_id() << " has " << unread.count
                  << " unread messages" << std::endl;
    }
    std::cout << std::endl;
 
    std::cout << "Unread messages for sender: " << sender_unread.size() << std::endl;
    for (auto unread : sender_unread) {
        std::cout << "Channel: " << unread.channel.channel_id() << " has " << unread.count
                  << " unread messages" << std::endl;
    }
    std::cout << std::endl;

    // You can always mark all messages as read for certain user 
    chat_sender.mark_all_messages_as_read();

    std::this_thread::sleep_for(std::chrono::seconds(3));
    auto sender_read = chat_sender.get_unread_messages_counts();

    std::cout << "Unread messages for sender after reading all: " << sender_read.size() << std::endl;
    for (auto unread : sender_read) {
        std::cout << "Channel: " << unread.channel.channel_id() << " has " << unread.count
                  << " unread messages" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "End of main" << std::endl;

    return 0;
}
