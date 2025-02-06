#ifndef PN_CHAT_C_RESPONSE_HPP
#define PN_CHAT_C_RESPONSE_HPP

#include "c_errors.hpp"
#include "c_utils.hpp"
#include "domain/quotes.hpp"

PnCResult pn_c_consume_response_buffer(char* result);

PnCResult pn_c_append_to_response_buffer(const char* data);

template <typename T>
PnCResult pn_c_append_pointer_to_response_buffer(Pubnub::String id, T* pointer) {
    auto id_string = Quotes::add(id);
    auto ptr_string = Quotes::add(to_ptr_string(pointer));

    return pn_c_append_to_response_buffer("{" + id_string + ":" + ptr_string + "}");
}

#endif
