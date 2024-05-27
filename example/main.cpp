#include "lib.hpp"
#include "c_functions/c_chat.hpp"
#include "string.hpp"
#include <iostream>

int main() {
    auto chat = pn_chat_new("demo", "demo", "user");

    pn_chat_send_message(chat, "Hello, World!");

    pn_chat_delete(chat);


    Pubnub::String string = "Hello, World!";

    std::cout << string.to_std_string() << std::endl;
}
