#include "c_functions/c_serialization.hpp"
#include "c_functions/c_channel.hpp"
#include "c_functions/c_chat.hpp"
#include "chat.hpp"
#include "chat/message.hpp"
#include "enums.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include "infra/pubnub.hpp"

int main() {
    std::string pub_key = "pub-c-2451c2cf-9f04-446b-8f85-e06564095e55";
    std::string sub_key = "sub-c-d16ff59f-b415-4ef9-8c29-ddda64fa2b43";
    std::string user = "hehehe";

    Pubnub::Chat chat(pub_key, sub_key, user);

    chat.get_pubnub_context().subscribe_to_channel("test_channel");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    chat.get_pubnub_context().subscribe_to_channel("test_channel2");

    std::this_thread::sleep_for(std::chrono::seconds(5));
}
