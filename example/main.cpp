#include "c_functions/c_channel.hpp"
#include "c_functions/c_chat.hpp"
#include "c_functions/c_user.hpp"
#include "chat.hpp"
#include "chat/message.hpp"
#include "chat/membership.hpp"
#include "chat/user.hpp"
#include "chat/channel.hpp"
#include "enums.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    
    std::string pub_key = "pub-c-79961364-c3e6-4e48-8d8d-fe4f34e228bf";
    std::string sub_key = "sub-c-2b4db8f2-c025-4a76-9e23-326123298667";
    std::string user = "huehue_hue_v4";

    auto chat = pn_chat_new(pub_key.c_str(), sub_key.c_str(), user.c_str());

    auto channel = pn_chat_create_public_conversation_dirty(chat, "test", "", "", "", "", "", "");

    char fake[1024];
    pn_channel_join(channel, "{}", fake);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto user_obj = pn_chat_get_user(chat, "huehue_hue_v4");

    std::this_thread::sleep_for(std::chrono::seconds(3));

    char buffer[1024];
    auto resp = pn_user_where_present(user_obj, buffer);

    std::cout << "Where am I? " << buffer << std::endl;
}
