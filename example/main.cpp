#include "lib.hpp"

int main() {
    Library lib;
    lib.publish("my_channel", "\"Hello world!\"");
    return 0;
}
