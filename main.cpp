// basic input, output handling
// error input handling
// C++14 is minimum required for this project
extern "C" {
#include <stdio.h>
}
#include <cstring>
#include <fstream>
#include "errors.h" // includes string
#include "lexer.h"
#include "parser.h"

std::string FindExtension(const char *input) noexcept {
    std::string output {};
    size_t i = 0;
    bool found = false;
    for (; input[i] != '\0'; ++i) {
        if (input[i] == '.') {
            found = true;
            ++i; // skip dot
            break;
        }
    }
    if (found) {
        while (input[i] != '\0') {
            output += input[i];
            ++i;
        }
    }
    return output;
}
std::string FileName(const std::string &input) {
    if (input.find(".") != std::string::npos) {
        size_t index = input.find(".");
        return input.substr(0, index);
    }
    return input;
}
ErrorType Report;
int main(int argc, char *argv[]) {
    if (argc == 1) {
        Report.NoInput();
        return 1;
    }
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        puts("Usage: mclang [File.mc]\n");
        puts("Compiler flags:");
        puts("\t-h\tDisplay this information.");
        puts("\t-v\tDisplay version of the compiler.\n");
        puts("\t-o\tSet name of the output to provided.");
        puts("\t-a\tOnly assemble, but not compile or link.");
        puts("\t-s\tAssemble and compile, but not link.");
        puts("\t-c\tAssemble, compile, link, but not remove build files.");
        puts("\t-e\tExecute code after compilation.");
        puts("\t-g\tCompile code with debugging symbols."); 
        return 0;
    }
    else if (argc == 2 && strcmp(argv[1], "-v") == 0) {
        puts("MC programming language - Version Beta 01");
        return 0;
    }
    std::string output = {};
    std::string input {};
    char flag = ' ';
    for (unsigned short i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0 && i+1 != argc) {
            output = argv[i+1];
        }
        else if (strcmp(argv[i], "-a") == 0) {
            flag = 'a';
        }
        else if (strcmp(argv[i], "-s") == 0) {
            flag = 's';
        }
        else if (strcmp(argv[i], "-c") == 0) {
            flag = 'c';
        }
        else if (strcmp(argv[i], "-e") == 0) {
            flag = 'e';
        }
        else if (strcmp(argv[i], "-g") == 0) {
            flag = 'g';
        }
        if (FindExtension(argv[i]) == "mc" && output.empty()) {
            input = argv[i];
            output = FileName(input);
        }
    }
    if (input.empty()) {
        Report.InputNotValid();
        return 1;
    }

    std::string nasm_cmds = Read(std::move(input));
    if (nasm_cmds.empty()) {
        return 1;
    }
    
    if (Compile(nasm_cmds, output, flag) == false) {
        return 1;
    }
    return 0;
}
