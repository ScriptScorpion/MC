#include <fstream>
#include <cstring>
#include <iostream>
#include "parser.h"

int main(int argc, char *argv[]) {
    if (argc <= 1 || argc > 7) {
        std::cout << "Usage: " << argv[0] << " <Options>\n";
        std::cout << "Options:\n" << 
        "-a (x64 or x32): Specify for what architecture of CPU the code is written for\n"
        "-i string: Specify input filename to read from\n"
        "-o string: Specify filename to write output into (default output file is OUT_NAME in parser.cpp )\n";
        return 0;
    }
    Parser parser;
    std::string arch {};
    std::string input {};
    std::string output {};
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            arch = argv[i+1];
            ++i;
        }
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input = argv[i+1];
            ++i;
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output = argv[i+1];
            ++i;
        }
        else {
            std::cerr << "Error: unknown option " << argv[i] << std::endl;
            return 1;
        }
    }
    if (arch.empty() || (arch != "x64" && arch != "x32")) {
        std::cerr << "Error: unknown architecture of the CPU" << std::endl;
        return 1;
    }
    if (!parser.correct_filename(input)) {
        std::cerr << "Error: incorrectly written input filename or not specified" << std::endl;
        return 1;
    }
    if (!parser.correct_filename(output)) {
        std::cerr << "Error: incorrectly written output filename" << std::endl;
        return 1;
    }
    std::string what {};
    std::string errors = parser.parse(input, output, arch);
    if (!errors.empty()) {
        std::cerr << "Error: " << errors << std::endl;
        return 1;
    }

    return 0;
}
