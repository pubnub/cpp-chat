#ifndef LIB_HPP
#define LIB_HPP

#include <string>
#include "export.hpp"

class Library {
    public:
        DLL_EXPORT int publish(std::string channel, std::string message);
};

DLL_EXPORT int publish_fn(std::string channel, std::string message);

DLL_EXPORT int publish_simple_fn();

#endif /* LIB_HPP */
