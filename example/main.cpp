#include "lib.hpp"
#include "c_functions/c_chat.hpp"
#include "string.hpp"
#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;
    Pubnub::String kekw("KEKW");
    Pubnub::String string = "Hello, World!";

    std::cout << kekw << std::endl;
    std::cout << kekw.c_str() << std::endl;
    std::cout << kekw.to_std_string() << std::endl;
    std::cout << string << std::endl;
    std::cout << string.c_str() << std::endl;
    std::cout << string.to_std_string() << std::endl;
}
