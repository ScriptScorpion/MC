// main error handling
extern "C" {
#include <stdio.h>
}
#include "errors.h" // string types are not needed in this file
#include "error_codes.h"
void ErrorType::NoInput() noexcept {
     printf("%s No input file found (add -h flag for help)\n", ErrorCode[0]);
     fflush(stdout);
}
void ErrorType::InputNotValid() noexcept {
    printf("%s File don't meet requirements\n", ErrorCode[0]);
    fflush(stdout);
}
void ErrorMsg::PrintError(const ErrorMsg &obj) noexcept {
    printf("%s %s\n", ErrorCode[obj.ErrorID], obj.Message.c_str());
}
