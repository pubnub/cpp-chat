#include "lib.hpp"

int main() {
    Library lib;
    lib.publish("my_channel", "\"Hello world!\"");

    publish_simple_fn();

    library_publish(&lib, "my_channel", "\"Hello world!\"");
    return 0;
}
