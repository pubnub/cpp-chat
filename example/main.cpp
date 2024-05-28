#include "infra/pubnub.hpp"
#include <iostream>
#include <thread>

int main() {
    PubNub pn("demo", "demo", "demo");

    pn.publish("my_channel", "\"Hello, world!\"");

    pn.subscribe_to_channel("my_channel");

    for (auto i = 0; i < 10; i++) {
        auto messages = pn.fetch_messages();
        for (const auto& message : messages) {
            std::cout << message.payload.ptr << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
