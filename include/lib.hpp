#ifndef LIB_HPP
#define LIB_HPP

#include <string>
#include "export.hpp"
#include "extern.hpp"

PN_CHAT_EXPORT class Library {
    public:
        PN_CHAT_EXPORT int publish(const char* channel, const char* message);
};

PN_CHAT_EXTERN PN_CHAT_EXPORT Library* library_create();

PN_CHAT_EXTERN PN_CHAT_EXPORT void library_destroy(Library* lib);

PN_CHAT_EXTERN PN_CHAT_EXPORT int library_publish(Library* lib, const char* channel, const char* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT int publish_fn(const char* channel, const char* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT int publish_simple_fn();

#endif /* LIB_HPP */
