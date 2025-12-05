// compiles tokens that correspond to commands
extern "C" {
#include <stdio.h>
}
#include <fstream>
#include <cstdlib>
#include "errors.h" // includes string

#ifdef __unix__
    const char *format = " -f elf64 ";
    const char *extraLD = "";
#else
    const char *format = " -f win64 ";
    const char *extraLD = " -subsystem console -entry _start"
#endif
void RemoveExtension(std::string &output_file) noexcept {
    if (output_file.find(".") != std::string::npos) {
        size_t index = output_file.find(".");
        output_file = output_file.substr(0, index);
    }
}
bool Compile(const std::string &nasm_cmds, std::string &output_file, const char &flag) {
    ErrorMsg ManualReport;
    RemoveExtension(output_file);
    std::ofstream output_f((output_file + ".asm"), std::ios::out);
    if (output_f.bad()) {
        ManualReport.ErrorID = 0;
        ManualReport.Message = "Cannot create a file";
        ManualReport.PrintError(ManualReport);
        return false;
    }
    for (const char &c : nasm_cmds) {
        output_f.put(c);
    }
    output_f.close();
    std::string exe_str {};
    switch (flag) {
        case ' ': {
            exe_str = "nasm" + std::string(format) + output_file + ".asm" + " -o " + output_file + ".o";
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot compile the code";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            exe_str = "ld " + output_file + ".o" " -o " + output_file + std::string(extraLD);
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot link the code";
                ManualReport.PrintError(ManualReport);

                return false;
            }
            exe_str = "rm " + output_file + ".asm " + output_file + ".o"; 
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot remove build files";
                ManualReport.PrintError(ManualReport);

                return false;
            }
            break;
        }
        case 'a': {
            return true;
            break;
        }
        case 's': {
            exe_str = "nasm" + std::string(format) + output_file + ".asm" + " -o " + output_file + ".o";
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot compile the code";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            break;
        }
        case 'c': {
            exe_str = "nasm" + std::string(format) + output_file + ".asm" + " -o " + output_file + ".o";
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot compile the code";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            exe_str = "ld " + output_file + ".o" " -o " + output_file + std::string(extraLD);
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot link the code";
                ManualReport.PrintError(ManualReport);

                return false;
            }
            break;
        }
        case 'e': {
            exe_str = "nasm" + std::string(format) + output_file + ".asm" + " -o " + output_file + ".o";
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot compile the code";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            exe_str = "ld " + output_file + ".o" " -o " + output_file + std::string(extraLD);
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot link the code";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            exe_str = "rm " + output_file + ".asm " + output_file + ".o"; 
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot remove build files";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            #ifdef __unix__
                exe_str = "./" + output_file;
            #else
                exe_str = ".\\" + output_file;
            #endif
            int status = system(exe_str.c_str());
            printf("\nError code: %i\n", WEXITSTATUS(status));
            fflush(stdout);
            break;
        }
        case 'g': {
            exe_str = "nasm -g" + std::string(format) + output_file + ".asm" + " -o " + output_file + ".o";
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot compile the code";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            exe_str = "ld -g " + output_file + ".o" " -o " + output_file + std::string(extraLD);
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot link the code";
                ManualReport.PrintError(ManualReport);

                return false;
            }
            exe_str = "rm " + output_file + ".asm " + output_file + ".o"; 
            if (system(exe_str.c_str()) != 0) {
                ManualReport.ErrorID = 0;
                ManualReport.Message = "Cannot remove build files";
                ManualReport.PrintError(ManualReport);

                return false;
            }
            break;
        }
    }
    return true;
}
