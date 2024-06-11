#include <cstring>
#include "c_functions/c_errors.hpp"

static char* pn_c_error_message_ptr = nullptr;

void clear_error_message() {
        delete[] pn_c_error_message_ptr;
        pn_c_error_message_ptr = nullptr;
}

void pn_c_get_error_message(char* buffer) {
   if (pn_c_error_message_ptr != nullptr) {
        strcpy(buffer, pn_c_error_message_ptr);

        clear_error_message();
   }
}

void pn_c_set_error_message(const char* message) {
    if (pn_c_error_message_ptr != nullptr) {
        clear_error_message();
    }

    pn_c_error_message_ptr = new char[strlen(message) + 1];
    strcpy(pn_c_error_message_ptr, message);
}
