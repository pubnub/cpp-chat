#include <string>

class Library {
    public:
        __declspec(dllexport) int publish(std::string channel, std::string message);
};

__declspec(dllexport) int publish_fn(std::string channel, std::string message);

__declspec(dllexport) int publish_simple_fn();
