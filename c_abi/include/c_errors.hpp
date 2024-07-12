#ifndef PN_C_ERRORS_HPP
#define PN_C_ERRORS_HPP

#include "helpers/export.hpp"
#include "helpers/extern.hpp"

typedef int PnCResult;
#define PN_C_OK 0
#define PN_C_ERROR -1
#define PN_C_ERROR_PTR nullptr

typedef int PnCTribool;
#define PN_C_TRUE 1
#define PN_C_FALSE 0
#define PN_C_UNKNOWN -1

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_c_get_error_message(char* buffer);

void pn_c_set_error_message(const char* message); 

#endif // PN_C_ERRORS_HPP
