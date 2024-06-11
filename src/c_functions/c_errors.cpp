#include <cstring>
#include "c_functions/c_errors.hpp"

static char* pn_c_error_message_ptr = nullptr;

void pn_c_set_error_ptr(char* ptr) {
    pn_c_error_message_ptr = ptr;
}

void pn_c_set_error_message(const char* message) {
    if (pn_c_error_message_ptr != nullptr) {
        //memcpy(pn_c_error_message_ptr, message, strlen(message) + 1);
        strcpy(pn_c_error_message_ptr, message);
    }
}
