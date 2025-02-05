#include "c_callback_handle.hpp"

void pn_callback_handle_dispose(Pubnub::CallbackHandle* handle) {
    delete handle;
}

void pn_callback_handle_close(Pubnub::CallbackHandle* handle) {
    handle->close();
}

