#include "c_response.hpp"
#include "infra/sync.hpp"
#include "string.hpp"
#include "c_errors.hpp"

static Mutex<Pubnub::String> buffer = "[";

PnCResult pn_c_consume_response_buffer(char *result) {
    try {
        auto lock = buffer.lock();

        *lock += "]";
        strcpy(result, lock->c_str());
    
        lock->clear();
        *lock += "[";
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what()); 

        return PN_C_ERROR;
    }
    
    return PN_C_OK;
}

PnCResult pn_c_append_to_response_buffer(const char* data) {
    try {
        auto lock = buffer.lock();
        if (lock->length() > 1) {
            *lock += ",";
        }

        *lock += data;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what()); 

        return PN_C_ERROR;
    }
    
    return PN_C_OK;
}
