#pragma once
#include <string>
struct ErrorType {
    void NoInput() noexcept;
    void InputNotValid() noexcept;
};
struct ErrorMsg {
    unsigned short ErrorID {};
    std::string Message {};
    void PrintError(const ErrorMsg &obj) noexcept;
};
