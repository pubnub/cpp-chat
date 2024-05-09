#include <string>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__) || defined(__MINGW32__)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif


class Library {
    public:
        DLL_EXPORT int publish(std::string channel, std::string message);
};

DLL_EXPORT int publish_fn(std::string channel, std::string message);

DLL_EXPORT int publish_simple_fn();
