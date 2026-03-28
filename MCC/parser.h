#pragma once
#include <string>
struct Parser {
    std::string parse(const std::string &input, const std::string &output, const std::string &arch);
    bool correct_filename(const std::string &str);
    bool have_spaces(const std::string &str);
};
