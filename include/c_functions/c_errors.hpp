#ifndef PN_C_ERRORS_HPP
#define PN_C_ERRORS_HPP

#include "export.hpp"
#include "extern.hpp"

typedef int PnCResult;

#define PN_C_OK 0
#define PN_C_ERROR -1
#define PN_C_ERROR_PTR nullptr

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_c_set_error_ptr(const char* ptr);

void pn_c_set_error_message(const char* message); 

#endif // PN_C_ERRORS_HPP
