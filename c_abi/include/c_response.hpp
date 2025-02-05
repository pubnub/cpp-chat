#ifndef PN_CHAT_C_RESPONSE_HPP
#define PN_CHAT_C_RESPONSE_HPP

#include "c_errors.hpp"

PnCResult pn_c_consume_response_buffer(char* result);

PnCResult pn_c_append_to_response_buffer(const char* data);

#endif
