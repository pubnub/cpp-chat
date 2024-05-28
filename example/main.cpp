#include "lib.hpp"
#include "c_functions/c_chat.hpp"
#include "string.hpp"
#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;
    Pubnub::String kekw("KEKW");
    Pubnub::String string = "Hello, World!";

    Pubnub::String chuj;


    chuj = string;

    auto xyz = Pubnub::String("abc") + Pubnub::String("def");

    std::cout << kekw << std::endl;
    std::cout << kekw.c_str() << std::endl;
    std::cout << kekw.to_std_string() << std::endl;
    std::cout << string << std::endl;
    std::cout << string.c_str() << std::endl;
    std::cout << string.to_std_string() << std::endl;
    std::cout << chuj << std::endl;
    std::cout << chuj.c_str() << std::endl;
    std::cout << chuj.to_std_string() << std::endl;
    std::cout << xyz << std::endl;
}
