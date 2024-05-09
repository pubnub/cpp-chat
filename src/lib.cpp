#include <iostream>
#include "lib.hpp"

extern "C" {
#include "core/pubnub_alloc.h"
#include "core/pubnub_pubsubapi.h"
#include "core/pubnub_ntf_sync.h"
}

int Library::publish(const char* channel, const char* message) {
    pubnub_t *ctx = pubnub_alloc();
    if (NULL == ctx) {
        std::cout << "Failed to allocate Pubnub context" << std::endl;
        return -1;
    }

    pubnub_init(ctx, "demo", "demo");
    pubnub_set_user_id(ctx, "my_unique_user_id");

    pubnub_publish(ctx, channel, message);
    auto res = pubnub_await(ctx);

    if (res != PNR_OK) {
        std::cout << "Failed to publish message" << std::endl;
        std::cout << "Error message: " << pubnub_last_publish_result(ctx) << std::endl;
    }
    
    pubnub_free(ctx);

    return res == PNR_OK ? 0 : -1;
}

int publish_fn(const char* channel, const char* message) {
    Library lib;
    return lib.publish(channel, message);
}

int publish_simple_fn() {
    Library lib;
    return lib.publish("my_channel", "\"Hello, world!\"");
}

Library* library_create() {
    return new Library();
}

void library_destroy(Library* lib) {
    if (lib != nullptr)
    {
        delete lib;
        lib = nullptr;
    }
}

int library_publish(Library* lib, const char* channel, const char* message) {
    if (lib != nullptr)
    {
        return lib->publish(channel, message);
    }

    return -1;
}
