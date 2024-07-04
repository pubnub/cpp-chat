#include "presentation/chat.hpp"
#include "presentation/channel.hpp"
#include "presentation/user.hpp"

#include "enums.hpp"
#include <chrono>
#include <string>
#include <iostream>
#include <thread>

int main() {
    
    std::string pub_key = "pub-c-79961364-c3e6-4e48-8d8d-fe4f34e228bf";
    std::string sub_key = "sub-c-2b4db8f2-c025-4a76-9e23-326123298667";
    std::string user = "hehehe";

    Pubnub::Chat chat(pub_key.c_str(), sub_key.c_str(), user.c_str());
    Pubnub::Channel channel = chat.create_public_conversation("asasa", Pubnub::ChatChannelData());


}
