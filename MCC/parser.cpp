#include <vector>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include "parser.h"

#define PREFIX true // you can change this one
#define GROUPBY 1 // you can change this one

const std::string TEMP_NAME = "mcc_f.bin";
const std::string TEMP_NAME2 = "mcc_f.o";
std::string OUT_NAME = "mcc_f";

bool Parser::correct_filename(const std::string &str) {
    if (str.empty()) {
        return true;
    }   
    for (const char &c : str) {
        if (std::ispunct(c) && c != '.') { 
            return false;
        }
    }   
    return true;
}
bool Parser::have_spaces(const std::string &str) {
    for (const char &c : str) {
        if (std::isspace(c)) {
            return true;
        }
    }
    return false;
}
std::string Parser::parse(const std::string &input, const std::string &output, const std::string &arch) {
    std::ifstream Reader(input);
    std::ofstream TempOut(TEMP_NAME);
    if (!Reader || !TempOut) {
        return "cannot open files";
    }
    if (!output.empty()) {
        OUT_NAME = output;
    }

    std::vector <std::string> insiders {};
    char c {};
    std::string temp {};
    int digit_temp {}; 
    while (Reader.get(c)) {
        if (c == '\n' || c == ' ') {
            insiders.push_back(temp);
            temp.assign('\0', temp.length());
            temp.clear();
        }
        else if (!std::isxdigit(static_cast<unsigned char>(c)) && ((PREFIX == true) ? c != 'x' : true)) {
            Reader.close();
            TempOut.close();
            remove(TEMP_NAME.c_str());
            return "your string can only be in hexadecimal format";
        }

        else {
            temp += std::tolower(c);
        }
    }
    if (PREFIX) {
        for (std::string &s : insiders) {
            if (s.substr(0, 2) == "0x" && (s.length() % 2) == 0) {
                s.erase(0, 2);
                if (GROUPBY * 2 > s.length()) {
                    Reader.close();
                    TempOut.close();
                    remove(TEMP_NAME.c_str());
                    return "you cannot specify number that is so big";
                }
                for (size_t i = 0; i < s.length(); i += GROUPBY * 2) {
                    digit_temp = std::stoi((s.substr(i, GROUPBY*2).c_str()), nullptr, 16);
                    if (digit_temp == 0) {
                        char arr_temp[GROUPBY];
                        memset(arr_temp, '\0', sizeof(arr_temp));
                        TempOut.write(arr_temp, sizeof(arr_temp));
                    }
                    else {
                        TempOut.write(reinterpret_cast<char*>(&digit_temp), strlen(reinterpret_cast<char*>(&digit_temp)));
                    }
                }
            }
            else {
                Reader.close();
                TempOut.close();
                remove(TEMP_NAME.c_str());
                return "check if your hex string is written correct";
            }
        }
    }
    else {
        for (std::string &s : insiders) {
            if (s.length() % 2 == 0) {
                if (GROUPBY * 2 > s.length()) {
                    return "you cannot specify number that is so big";
                }
                for (size_t i = 0; i < s.length(); i += GROUPBY * 2) {
                    digit_temp = std::stoi((s.substr(i, GROUPBY*2).c_str()), nullptr, 16);
                    if (digit_temp == 0) {
                        char arr_temp[GROUPBY];
                        memset(arr_temp, '\0', sizeof(arr_temp));
                        TempOut.write(arr_temp, sizeof(arr_temp));
                    }
                    else {
                        TempOut.write(reinterpret_cast<char*>(&digit_temp), strlen(reinterpret_cast<char*>(&digit_temp)));
                    }
                }
            }
            else {
                Reader.close();
                TempOut.close();
                remove(TEMP_NAME.c_str());
                return "check if your hex string is written correct";
            }
        }
    }
    Reader.close();
    TempOut.close();

    #ifdef __unix__
        if (arch == "x64") {
            temp = "objcopy -I binary -O elf64-x86-64 --rename-section .data=.text " + TEMP_NAME + " " + TEMP_NAME2;
            system(temp.c_str());
            temp = "ld --no-warnings " + TEMP_NAME2 + " -o " + OUT_NAME;
            system(temp.c_str());
        }
        else {
            temp = "objcopy -I binary -O elf32-i386 --rename-section .data=.text " + TEMP_NAME + " " + TEMP_NAME2;
            system(temp.c_str());
            temp = "ld --no-warnings -m elf_i386 " + TEMP_NAME2 + " -o " + OUT_NAME;
            system(temp.c_str());
        }
    
    #else
        if (arch == "x64") {
            temp = "objcopy -I binary -O pe-x86-64 --rename-section .data=.text " + TEMP_NAME + " " + TEMP_NAME2;
            system(temp.c_str());
            temp = "ld --no-warnings " + TEMP_NAME2 + " -o " + OUT_NAME;
            system(temp.c_str());
        }
        else {
            temp = "objcopy -I binary -O pe-i386 --rename-section .data=.text " + TEMP_NAME + " " + TEMP_NAME2;
            system(temp.c_str());
            temp = "ld --no-warnings -m i386pe " + TEMP_NAME2 + " -o " + OUT_NAME;
            system(temp.c_str());
        }
    #endif
    remove(TEMP_NAME.c_str());
    remove(TEMP_NAME2.c_str());
    return "";
}
