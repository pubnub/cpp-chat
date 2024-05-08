#include <string>

class Library {
    public:
        int publish(std::string channel, std::string message);
};

int publish_fn(std::string channel, std::string message);

int publish_simple_fn();
