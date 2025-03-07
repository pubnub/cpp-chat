#include "const_values.hpp"
#include "enums.hpp"
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

    chat.listen_for_events(Pubnub::INTERNAL_MODERATION_PREFIX + channel.channel_id(), Pubnub::PCET_REPORT, [](const Pubnub::Event &event) {
        std::cout << "Received event: " << std::endl;
        std::cout << "Event type: " << Pubnub::chat_event_type_to_string(event.type) << std::endl;
        std::cout << "Event payload: "<< event.payload << std::endl;
    });

    channel.connect([&channel](Pubnub::Message message) {
        std::cout << "Received message: " << message.text() << std::endl;

        message.report("Some Reason why I've reported this message");

        std::this_thread::sleep_for(std::chrono::seconds(1));

        message.timetoken();
    });

    channel.send_text("This message is abusing the rules, please take a look at it");

    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "End of main" << std::endl;
}

