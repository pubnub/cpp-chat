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

    // Create channel that will publish message that will be the thread initializer
    auto channel = chat.create_public_conversation("base_channel", Pubnub::ChatChannelData{});

    // Create thread after receiving the message
    // All sleeps are related to the synchronization - in your code probably you can just be more
    // proactive and depend on the `stream_updates` methods
    channel.connect([&channel](const Pubnub::Message& message) {
            std::cout << "Message:" << message.text() << std::endl;

            // creation of thread entity
            auto thread = message.create_thread();

            std::this_thread::sleep_for(std::chrono::seconds(2));

            thread.connect([](const Pubnub::Message& message_inner) {
                    std::cout << "Thread Message:" << message_inner.text() << std::endl;
            });

            std::this_thread::sleep_for(std::chrono::seconds(2));

            // Send text to the thread and create it on PubNub infrastracture
            thread.send_text("first message of the thread");

            std::this_thread::sleep_for(std::chrono::seconds(4));

            // Update message because of the immutable entities
            auto updated_message = channel.get_message(message.timetoken());

            std::cout << "Has thread: " << updated_message.has_thread() << std::endl;

            // This is how you can get the thread if it is already created
            auto thread_get = updated_message.get_thread();

            std::cout << "Thread got: " << thread_get.channel_id() << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(2));

            // Thread removal - be careful - if thread is already deleted then 
            // it will throw exception
            updated_message.remove_thread();

            std::this_thread::sleep_for(std::chrono::seconds(2));

            auto message_after_removal = channel.get_message(message.timetoken());

            std::cout << "Has thread: " << message_after_removal.has_thread() << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
    // Send message
    channel.send_text("Start new thread from this message");

    std::this_thread::sleep_for(std::chrono::seconds(20));

    std::cout << "End of main" << std::endl;

    return 0;
}
