// split keyword into tokens(lexems)
// analyze tokens so that keywords match exatly as i expect
#include <memory>
#include <vector>
#include <tuple>
#include <fstream>
#include <cstring>
#include <cctype>
#include <climits>
#include "errors.h" // includes string
#include "cmds.h"
#include "hash_function.h"
#include "ops.h"

#define Addition 0
#define Subtraction 1
#define Multiplication 2
#define Division 3
#define Remainder 4
#define And 5
#define Or 6
#define Xor 7

#define EQ 0
#define NEQ 1
#define L 2
#define LE 3
#define G 4
#define GE 5

#define Int8_t 0
#define Int16_t 1
#define Int32_t 2
#define Int64_t 3
#define String8_t 4
#define String16_t 5
#define String32_t 6
#define String64_t 7  
#define RETURN 8
#define PRINT 9
#define PRINTLN 10
#define READ 11
#define GOTO 12
#define GOTO_IF 13
ErrorMsg ManualReport;

class Function_vals {
    public:
        bool Called(const std::string &name) const noexcept {
            for (auto &p : func_values) {
                if (std::get<0>(p) == name) { // example: 'func()' - correct syntax 'func ()' - not correct syntax
                    if (name.find(' ') != std::string::npos) { 
                        ManualReport.ErrorID = 2;
                        ManualReport.Message = "Incorrect syntax for calling function";
                        ManualReport.PrintError(ManualReport);
                        return false;
                    }
                    else {
                        return true;
                    }
                }
            }
            return false;
        }
        std::string Get_code(const std::string &name) const {
            for (auto &p : func_values) {
                if (std::get<0>(p) == name) {
                    return std::get<1>(p);
                }
            }
            std::exit(2); // Critical error
            return "";
        }
        std::string Get_Arguments(const std::string &name) const {
            for (auto &p : func_values) {
                if (std::get<0>(p) == name) {
                    return std::get<2>(p);
                }
            }
            std::exit(2); // Critical error
            return "";
        }
        void Add_function(const std::string &name, const std::string &code, const std::string &arguments) noexcept {
            func_values.push_back(std::tuple <std::string, std::string, std::string> (name, code, arguments));
        }
    private:
        //                       name            code       arguments
        std::vector <std::tuple<std::string, std::string, std::string>> func_values;
}Functions;

class Variable_vals {
    public:
        std::string Get_value(const std::string &name) const {
            for (auto &p : vars_values) {
                if (std::get<0>(p) == name) {
                    return std::get<1>(p);
                }
            }
            std::exit(2); // Critical error
            return "";
        }
        char Get_size(const std::string &name) const {
            for (auto &p : vars_values) {
                if (std::get<0>(p) == name) {
                    return std::get<2>(p);
                }
            }
            std::exit(2); // Critical error
            return ' ';
        }
        bool Exists(const std::string &name) const noexcept {
            for (auto &p : vars_values) {
                if (std::get<0>(p) == name) {
                    return true;
                }
            }
            return false;
        }
        void Change_value(const std::string &name, const std::string &value) noexcept {
            for (auto &p : vars_values) {
                if (std::get<0>(p) == name) {
                    std::get<1>(p) = value;
                    break;
                }
            }
        }
        void Add_value(const std::string &name, const std::string &value, const char &c) noexcept {
           vars_values.push_back(std::tuple <std::string, std::string, char> (name, value, c)); 
        }

        void Clear(const std::vector <std::string> &&names = {}) noexcept {
            bool not_remove = false;
            if (names.empty()) {
                vars_values.clear();
            }
            else {
                for (size_t i = 0; i < vars_values.size(); ++i) {
                   for (const std::string &s : names) {
                        if (s == std::get<0>(vars_values[i])) {
                            not_remove = true;
                            break;
                        }
                    }
                    if (not_remove) {
                        not_remove = false;
                    }
                    else {
                        vars_values.erase(vars_values.begin() + i);
                    }
                }
            }
        }
    private:
        //                        name          value         size
        std::vector <std::tuple<std::string, std::string, char>> vars_values;
}Variables;

bool SplitIntoWords(const std::string &sentence, std::vector <std::string> &words) noexcept {
    std::string word {};
    for (size_t i = 0; i <= sentence.length(); ++i) { // looking a little bit further to determine end of the string
        if ((std::isspace(sentence[i]) || sentence[i] == '\0') && !word.empty()) {
            if (std::isblank(sentence[i]) && (i + 1 <= sentence.length()) && std::isblank(sentence[i+1])) {
                ManualReport.ErrorID = 1;
                ManualReport.Message = "Cannot have more then 1 space";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            words.push_back(word);
            word.assign(word.length(), '\0');
            word.clear();
        }
        else if (std::isgraph(sentence[i]) && !std::iscntrl(sentence[i])) {
            word += sentence[i];
        }
    }
    return true;
}
bool IsInList(const char *input) noexcept {
    for (const char *cs : Commands) {
        if (strcmp(input, cs) == 0) {
            return true;
        }
    }
    return false;
}

bool IsDigit(const std::string &input) noexcept {
    if (input.empty()) {
        return false;
    }
    for (const char &c : input) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}
bool IsString(const std::string &input) noexcept {
    if (input.empty()) {
        return false;
    }
    if (IsDigit(input)) {
        return false;
    }
    for (const char &c : input) {
        if (!std::isalnum(c)) {
            return false; 
        }
    }
    return true;
}

bool IsSpecialString(const std::string &input) noexcept {
    if (input.front() != '\"' || input.back() != '\"') {
        return false;
    }
    return true;
}
bool IsVariable(const std::string &input) noexcept {
    if (input.front() != '(' || input.back() != ')') {
        return false;
    }
    return true;
}
bool IsCorrectArguments(const std::string &arguments) noexcept {
    if (arguments == " ") {
        return true;
    }
    std::vector <std::string> vars {};
    std::string word {};
    for (size_t i = 0; i < arguments.length(); ++i) {
        if (arguments[i] == ',' && std::isblank(arguments[i+1]) && i+1 <= arguments.length()) {
            vars.push_back(word);
            word.assign(word.length(), '\0');
            word.clear();
            ++i; // skip space
        }
        else {
            word += arguments[i];
        }
    }
    vars.push_back(word); // adding last word
    if (vars.size() > 1 && arguments.find(',') != std::string::npos) {
        return true;
    }
    else if (vars.size() == 1 && arguments.find(',') == std::string::npos && arguments.find(' ') == std::string::npos) {
        return true;
    }
    return false;
}
bool IsCorrectSyntax(const std::string &input) noexcept {
    bool found_chars = false;
    for (const char &c : input) {
        if (std::isblank(c) && found_chars == false) {
            return false;
        }
        else if (std::isblank(c) && found_chars) {
            found_chars = false;
        }
        else if (std::isalnum(c)) {
            found_chars = true;
        }
    }
    return true;
}
bool FindStart(std::unique_ptr <char[]> &buffer) {  
    std::string fun_name {};
    std::string arguments {}; 
    bool wrong_place = false;
    bool foundcb = false;
    bool found_pars = false;
    size_t first_pos {0}; 
    size_t last_pos {0}; 
    for (size_t i = 0; buffer[i] != '\0'; ++i) {
        if (buffer[i] == '(' && (i+1 <= strlen(buffer.get())) && found_pars == false) {
            for (size_t j = 0; j < fun_name.length(); ++j) {
                if (std::isblank(fun_name[j]) && std::isalnum(fun_name[j+1]) && j+1 <= fun_name.length()) {
                    wrong_place = true;
                    break;
                }
            }
            if (wrong_place) {
                ManualReport.ErrorID = 1;
                ManualReport.Message = "Parenthesis for function is not found";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            if (buffer[i+1] != ')')  {
                ++i; // skip to what after bracket
                while(buffer[i] != '\n' && buffer[i] != '\v' && i < strlen(buffer.get())) {
                    if (buffer[i] == ')') {
                        found_pars = true;
                        break;
                    }
                    arguments += buffer[i];
                    ++i;
                }
                if (found_pars == false) {
                    break;
                }
            }
            else {
                arguments = " ";
                found_pars = true;
            }
        }
        else if (buffer[i] == ')' && found_pars == false) {
            fun_name = "start "; // needed so you get correct error message
            break;
        }
        else if (buffer[i] == '{' && (i+1 < strlen(buffer.get())) && foundcb == false && found_pars) {
            size_t j = 1;
            while (i+j <= strlen(buffer.get()) && std::iscntrl(buffer[i+j])) {
                ++j;
            }
            first_pos = i + j;
            while(buffer[i] != '\0') {
                if (buffer[i] == '}') {
                    foundcb = true;
                    last_pos = i - 1;
                    break;
                }
                ++i;
            }
            if (foundcb == false) {
                break;
            }
    
            std::string code {}; 
            for (size_t t = 0; t < (last_pos - first_pos + 1); ++t) {
                code += buffer[first_pos + t]; 
            }
            if (fun_name == "start ") {
                if (arguments != " ") {
                    ManualReport.ErrorID = 1;
                    ManualReport.Message = "Start function cannot have arguments";
                    ManualReport.PrintError(ManualReport);
                    return false;
                }
                break;
            }
            if (IsCorrectSyntax(fun_name) == false) {
                ManualReport.ErrorID = 1;
                ManualReport.Message = "Functions can only have 1 space";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            if (IsCorrectArguments(arguments) == false) {
                ManualReport.ErrorID = 1;
                ManualReport.Message = "Incorrect syntax for arguments declaration";
                ManualReport.PrintError(ManualReport);
                return false;
            }
            // (substr working not inclusive last character)
            Functions.Add_function(fun_name.substr(0, fun_name.length()-1), code, arguments); // was 'func ' become 'func' after substr
            fun_name.assign(fun_name.length(), '\0');
            fun_name.clear();
            code.assign(code.length(), '\0');
            code.clear();
            foundcb = false;
            found_pars = false;
            wrong_place = false;
            arguments.assign(arguments.length(), '\0');
            arguments.clear();
            first_pos = 0;
            last_pos = 0;
            continue;
        }
        else if (buffer[i] == '}' && foundcb == false) {
            break;
        }
        // Look into it if new version of this have bugs, if so can change to arguments.empty()
        else if (found_pars == false && !std::iscntrl(buffer[i])) { // not done !std::isspace because of syntax
            fun_name += buffer[i];
        }
    }
    if (fun_name == "start " && found_pars && foundcb) {
        std::unique_ptr <char[]> copy_buffer = std::make_unique <char[]> (last_pos - first_pos + 2); // +2(+1 cuz of index, +1 cuz of end character)
        size_t j = 0;
        for (; j <= (last_pos - first_pos); ++j) {
            copy_buffer[j] = buffer[first_pos + j];
        }
        copy_buffer[j] = '\0';
        buffer.reset();
        buffer = std::make_unique <char[]> (last_pos - first_pos + 2);
        strcpy(buffer.get(), copy_buffer.get());
    }
    else if (fun_name != "start ") {
        ManualReport.ErrorID = 1;
        ManualReport.Message = "Start function not found or written incorrectly";
        ManualReport.PrintError(ManualReport);
        return false;
    }
    else if (found_pars == false) {
        ManualReport.ErrorID = 1;
        ManualReport.Message = "Function doesn't have parenthesis";
        ManualReport.PrintError(ManualReport);
        return false;
    }
    else {
        ManualReport.ErrorID = 1;
        ManualReport.Message = "Function doesn't have curly braces";
        ManualReport.PrintError(ManualReport);
        return false;
    }
    return true;
}

bool Tokenize(const std::unique_ptr <char[]> &buffer, std::string &nasm_vars, std::string &nasm_arrs, std::string &nasm_cmds) {
    bool isfirst = true;
    std::vector <std::string> words {};
    std::string sentence {};
    for (size_t t = 0; buffer[t] != '\0'; ++t) {
        if (buffer[t] == '\n' || buffer[t] == '\v' || buffer[t] == '\0') {
            sentence += '\0';
            if (SplitIntoWords(sentence, words) == false) {
                return false;
            }
            if (words.empty()) {
                sentence.assign(sentence.length(), '\0');
                sentence.clear();
                continue;
            }
            // Main loop
            auto check_loop = [&]() -> bool {
                for (const char *s : Commands) {
                    if (strcmp(words[0].c_str(), s) < 0 || strcmp(words[0].c_str(), s) > 0) {
                        if (IsInList(words[0].c_str())) {
                            continue;
                        }
                        else if (Variables.Exists(words[0])) {
                            if (words.size() == 3) {
                                if (IsDigit(Variables.Get_value(words[0])) && IsDigit(words[2])) {
                                    switch (HashFunc(words[1].c_str())) {
                                        case HashFunc("="): {
                                            nasm_cmds += "\tmov rax, " + words[2] + "\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], words[2]);
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Addition]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tadd rax, " + words[2] + "\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) + std::stoull(words[2])));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Subtraction]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tsub rax, " + words[2] + "\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) - std::stoull(words[2])));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Multiplication]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\timul rax, " + words[2] + "\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) * std::stoull(words[2])));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Division]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tmov rbx, " + words[2] + "\n";
                                            nasm_cmds += "\txor rdx, rdx\n";
                                            nasm_cmds += "\tdiv rbx\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n"; // quotient in rax, remainder in rdx
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) / std::stoull(words[2])));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Remainder]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tmov rbx, " + words[2] + "\n";
                                            nasm_cmds += "\txor rdx, rdx\n";
                                            nasm_cmds += "\tdiv rbx\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rdx\n"; // quotient in rax, remainder in rdx
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) % std::stoull(words[2])));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[And]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tand rax, " + words[2] + "\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) & std::stoull(words[2])));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Or]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tor rax, " + words[2] + "\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) | std::stoull(words[2])));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Xor]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\txor rax, " + words[2] + "\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) ^ std::stoull(words[2])));
                                            return true;
                                        }
                                        default: {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Invalid operator: " + words[1];
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                    }
                                    
                                }
                                else if(IsSpecialString(Variables.Get_value(words[0])) && words[1] == "=" && IsSpecialString(words[2])) {
                                    if ((strlen(Variables.Get_value(words[0]).c_str())-2) >= (strlen(words[2].c_str())-2)) { // -2 cause of "" characters
                                        nasm_cmds += "\tmov rax, " + words[2] + "\n";
                                        nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                        Variables.Change_value(words[0], words[2]);
                                        return true;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "String variable cannot have larger size then already declared";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (IsDigit(Variables.Get_value(words[0])) && Variables.Exists(words[2])) { 
                                    if (IsSpecialString(Variables.Get_value(words[2]))) { // this works because variables need to be with quotes("") in initialization and assign operations
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "There can be no operations between integer variable and string variable";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    switch (HashFunc(words[1].c_str())) {
                                        case HashFunc("="): {
                                            nasm_cmds += "\tmov rax, [rel " + words[2] + "]\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], Variables.Get_value(words[2]));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Addition]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tmov rbx, [rel " + words[2] + "]\n";
                                            nasm_cmds += "\tadd rax, rbx\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) + std::stoull(Variables.Get_value(words[2]))));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Subtraction]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tmov rbx, [rel " + words[2] + "]\n";
                                            nasm_cmds += "\tsub rax, rbx\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) - std::stoull(Variables.Get_value(words[2]))));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Multiplication]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tmov rbx, [rel " + words[2] + "]\n"; 
                                            nasm_cmds += "\timul rax, rbx\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) * std::stoull(Variables.Get_value(words[2]))));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Division]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tmov rbx, [rel " + words[2] + "]\n";
                                            nasm_cmds += "\txor rdx, rdx\n";
                                            nasm_cmds += "\tdiv rbx\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n"; // quotient in rax, remainder in rdx
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) / std::stoull(Variables.Get_value(words[2]))));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Remainder]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tmov rbx, [rel " + words[2] + "]\n";
                                            nasm_cmds += "\txor rdx, rdx\n";
                                            nasm_cmds += "\tdiv rbx\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rdx\n"; // quotient in rax, remainder in rdx
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) % std::stoull(Variables.Get_value(words[2]))));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[And]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tmov rbx, [rel " + words[2] + "]\n";
                                            nasm_cmds += "\tand rax, rbx\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n"; 
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) & std::stoull(Variables.Get_value(words[2]))));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Or]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tmov rbx, [rel " + words[2] + "]\n";
                                            nasm_cmds += "\tor rax, rbx\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n"; 
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) | std::stoull(Variables.Get_value(words[2]))));
                                            return true;
                                        }
                                        case HashFunc(AssignOP[Xor]): {
                                            nasm_cmds += "\tmov rax, [rel " + words[0] + "]\n";
                                            nasm_cmds += "\tmov rbx, [rel " + words[2] + "]\n";
                                            nasm_cmds += "\txor rax, rbx\n";
                                            nasm_cmds += "\tmov [rel " + words[0] + "], rax\n"; 
                                            Variables.Change_value(words[0], std::to_string(std::stoull(Variables.Get_value(words[0])) ^ std::stoull(Variables.Get_value(words[2]))));
                                            return true;
                                        }
                                        default: {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Invalid operator: " + words[1];
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                    }
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Invalid operation with variable";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                            }
                            else if (words.size() > 3) {
                                if (IsSpecialString(Variables.Get_value(words[0])) && words[1] == "=") {
                                    if (words[words.size()-1] != "\"" || words[2] != "\"") {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable operation syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    size_t j = 3;
                                    std::string full_word {};
                                    while(words[j] != "\"" && j < words.size()-1) {
                                        full_word += words[j];
                                        if ((j+1) != words.size()-1) {
                                            full_word += ' ';
                                        }
                                        ++j;
                                    }
                                    words[2] = '\"' + full_word + '\"';
                                    words.resize(3);
                                    full_word.assign(full_word.length(), '\0');
                                    full_word.clear();
                                    nasm_cmds += "\tmov rax, " + words[2] + "\n";
                                    nasm_cmds += "\tmov [rel " + words[0] + "], rax\n";
                                    Variables.Change_value(words[0], words[2]);
                                    return true;
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too many arguments for operation with digit variable";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                            }
                            else {
                                ManualReport.ErrorID = 1;
                                ManualReport.Message = "Too few arguments for variable operation";
                                ManualReport.PrintError(ManualReport);
                                return false;
                            }
                        }
                        else if (words[0].find('(') != std::string::npos && Functions.Called(words[0].substr(0, words[0].find('(')))) {
                            std::string arguments {};
                            if (words[0].find(')') == std::string::npos) {
                                for (size_t i = 0; i < words.size(); ++i) {
                                       arguments += words[i];
                                       if (i+1 != words.size()) {
                                            arguments += " ";
                                       }
                                }
                                if (arguments.find(')') == std::string::npos) {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Second parenthesis not found";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                arguments.erase(0, arguments.find('(')+1); // removes all behind '(' including bracket itself
                                arguments.erase(arguments.length()-1, 1); // removes ')'
                            }
                            else {
                                arguments = " ";
                            }
                            words[0].erase(words[0].find('('), words[0].length());  // removes all except function name  
                            std::string func_arguments = Functions.Get_Arguments(words[0]);
                            nasm_cmds += words[0] + ": \n";
                            std::string temp1 = Functions.Get_code(words[0]);
                            std::unique_ptr <char[]> temp2 = std::make_unique <char[]> (temp1.length() + 1);
                            std::vector <std::string> clear_arguments {};
                            strcpy(temp2.get(), temp1.c_str());
                            if (!IsCorrectArguments(arguments)) {
                                ManualReport.ErrorID = 1;
                                ManualReport.Message = "Incorrect syntax for passing arguments";
                                ManualReport.PrintError(ManualReport);
                                return false;
                            }
                            else if (arguments != func_arguments) {
                                ManualReport.ErrorID = 1;
                                ManualReport.Message = "The arguments that you are provided, must match with the arguments defined in the function declaration";
                                ManualReport.PrintError(ManualReport);
                                return false;
                            } 
                            if (func_arguments != " ") { // i did that, because it will be bigger code if i do with .empty() case
                                std::string::size_type pos = func_arguments.find(',');
                                if (pos == std::string::npos) {
                                    clear_arguments.push_back(func_arguments);
                                }
                                while (pos != std::string::npos) {
                                    func_arguments.erase(pos, 1);
                                    pos = func_arguments.find(',');
                                }
                                for (size_t i = 0; i <= func_arguments.length(); ++i) { // looking a little bit further to determine end of the string
                                    if (std::isblank(func_arguments[i])) {
                                        clear_arguments.push_back(func_arguments.substr(0, i));
                                        func_arguments.erase(0, i+1);
                                        i = 0;
                                    }
                                    else if (func_arguments[i] == '\0') {
                                        clear_arguments.push_back(func_arguments.substr(0, i));
                                        break;
                                    }
                                }
                                for (std::string &s : clear_arguments) {
                                    if (!Variables.Exists(s)) {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "A non-existent variable has been passed";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                Variables.Clear(std::move(clear_arguments)); // this saves arguments only that are passed, by default clear all
                            }
                            else {
                                Variables.Clear();
                            }
                            if (Tokenize(temp2, nasm_vars, nasm_arrs, nasm_cmds)) {
                                return true;
                            }
                            else {
                                return false;
                            }
                        }
                        else {
                            ManualReport.ErrorID = 1;
                            ManualReport.Message = "Unknown command: " + words[0]; 
                            ManualReport.PrintError(ManualReport);
                            return false;
                        }
                    }
                    else {
                        switch (HashFunc(words[0].c_str())) {
                            case HashFunc(Commands[Int8_t]): {
                                if (words.size() == 4) {
                                    if (IsString(words[1]) && words[2] == "=" && IsDigit(words[3]) && !Variables.Exists(words[1])) {
                                        if (std::stoull(words[3]) <= UCHAR_MAX) {
                                            nasm_vars += "\t" + words[1] + ": db " + words[3] + "\n";
                                            Variables.Add_value(words[1], words[3], 'b');
                                            return true;
                                        }
                                        else {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Size overflow";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (words.size() > 4) {
                                    if (IsString(words[1]) && words[2] == "=" && !Variables.Exists(words[1])) {
                                        long long counter = 0;
                                        isfirst = true;
                                        for (size_t j = 3; j < words.size() - 1; ++j) { // -1 needed cuz of we go out the scope if we do so
                                            if (isfirst) {
                                                if (IsDigit(words[j-1]) && IsDigit(words[j+1]) && (j+1 <= words.size())) {
                                                    switch (HashFunc(words[j].c_str())) {
                                                        case HashFunc(Operator[Addition]): {
                                                            counter = std::stoi(words[j-1]) + std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Subtraction]): {
                                                            counter = std::stoi(words[j-1]) - std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Multiplication]): {
                                                            counter = std::stoi(words[j-1]) * std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Division]): {
                                                            counter = std::stoi(words[j-1]) / std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Remainder]): {
                                                            counter = std::stoi(words[j-1]) % std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[And]): {
                                                            counter = std::stoi(words[j-1]) & std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Or]): {
                                                            counter = std::stoi(words[j-1]) | std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Xor]): {
                                                            counter = std::stoi(words[j-1]) ^ std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect operation with integer variable";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false;
                                                        }
                                                    }
                                                }
                                            }
                                            else  {
                                                if (IsDigit(words[j+1]) && (j+1 <= words.size())) {
                                                    switch (HashFunc(words[j].c_str())) {
                                                        case HashFunc(Operator[Addition]): {
                                                            counter += std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Subtraction]): {
                                                            counter -= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Multiplication]): {
                                                            counter *= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Division]): {
                                                            counter /= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Remainder]): {
                                                            counter %= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[And]): {
                                                            counter &= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Or]): {
                                                            counter |= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Xor]): {
                                                            counter ^= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect operation with integer variable";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        if (counter > CHAR_MAX) {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Size overflow";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                        nasm_vars += "\t" + words[1] + ": db " + std::to_string(counter)  + "\n";
                                        Variables.Add_value(words[1], std::to_string(counter), 'b');
                                        return true;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too few arguments for variable creation";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[Int16_t]): {
                                if (words.size() == 4) {
                                    if (IsString(words[1]) && words[2] == "=" && IsDigit(words[3]) && !Variables.Exists(words[1])) {
                                        if (std::stoull(words[3]) <= USHRT_MAX) {
                                            nasm_vars += "\t" + words[1] + ": dw " + words[3] + "\n";
                                            Variables.Add_value(words[1], words[3], 'w');
                                            return true;
                                        }
                                        else {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Size overflow";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (words.size() > 4) {
                                    if (IsString(words[1]) && words[2] == "=" && !Variables.Exists(words[1])) {
                                        long long counter = 0;
                                        isfirst = true;
                                        for (size_t j = 3; j < words.size() - 1; ++j) {
                                            if (isfirst) {
                                                if (IsDigit(words[j-1]) && IsDigit(words[j+1]) && (j+1 <= words.size())) {
                                                    switch (HashFunc(words[j].c_str())) {
                                                        case HashFunc(Operator[Addition]): {
                                                            counter = std::stoi(words[j-1]) + std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Subtraction]): {
                                                            counter = std::stoi(words[j-1]) - std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Multiplication]): {
                                                            counter = std::stoi(words[j-1]) * std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Division]): {
                                                            counter = std::stoi(words[j-1]) / std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Remainder]): {
                                                            counter = std::stoi(words[j-1]) % std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[And]): {
                                                            counter = std::stoi(words[j-1]) & std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Or]): {
                                                            counter = std::stoi(words[j-1]) | std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Xor]): {
                                                            counter = std::stoi(words[j-1]) ^ std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect operation with integer variable";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false;
                                                        }
                                                    }
                                                }
                                            }
                                            else  {
                                                if (IsDigit(words[j+1]) && (j+1 <= words.size())) {
                                                    switch (HashFunc(words[j].c_str())) {
                                                        case HashFunc(Operator[Addition]): {
                                                            counter += std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Subtraction]): {
                                                            counter -= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Multiplication]): {
                                                            counter *= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Division]): {
                                                            counter /= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Remainder]): {
                                                            counter %= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[And]): {
                                                            counter &= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Or]): {
                                                            counter |= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Xor]): {
                                                            counter ^= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect operation with integer variable";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        if (counter > SHRT_MAX) {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Size overflow";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                        nasm_vars += "\t" + words[1] + ": dw " + std::to_string(counter)  + "\n";
                                        Variables.Add_value(words[1], std::to_string(counter), 'w');
                                        return true;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too few arguments for variable creation";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[Int32_t]): {
                                if (words.size() == 4) {
                                    if (IsString(words[1]) && words[2] == "=" && IsDigit(words[3]) && !Variables.Exists(words[1])) {
                                        if (std::stoull(words[3]) <= ULONG_MAX) {
                                            nasm_vars += "\t" + words[1] + ": dd " + words[3] + "\n";
                                            Variables.Add_value(words[1], words[3], 'd');
                                            return true;
                                        }
                                        else {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Size overflow";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }  
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (words.size() > 4) {
                                    if (IsString(words[1]) && words[2] == "=" && !Variables.Exists(words[1])) {
                                        long long counter = 0;
                                        isfirst = true;
                                        for (size_t j = 3; j < words.size() - 1; ++j) {
                                            if (isfirst) {
                                                if (IsDigit(words[j-1]) && IsDigit(words[j+1]) && (j+1 <= words.size())) {
                                                    switch (HashFunc(words[j].c_str())) {
                                                        case HashFunc(Operator[Addition]): {
                                                            counter = std::stoi(words[j-1]) + std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Subtraction]): {
                                                            counter = std::stoi(words[j-1]) - std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Multiplication]): {
                                                            counter = std::stoi(words[j-1]) * std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Division]): {
                                                            counter = std::stoi(words[j-1]) / std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Remainder]): {
                                                            counter = std::stoi(words[j-1]) % std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[And]): {
                                                            counter = std::stoi(words[j-1]) & std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Or]): {
                                                            counter = std::stoi(words[j-1]) | std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Xor]): {
                                                            counter = std::stoi(words[j-1]) ^ std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect operation with integer variable";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false;
                                                        }
                                                    }
                                                }
                                            }
                                            else  {
                                                if (IsDigit(words[j+1]) && (j+1 <= words.size())) {
                                                    switch (HashFunc(words[j].c_str())) {
                                                        case HashFunc(Operator[Addition]): {
                                                            counter += std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Subtraction]): {
                                                            counter -= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Multiplication]): {
                                                            counter *= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Division]): {
                                                            counter /= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Remainder]): {
                                                            counter %= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[And]): {
                                                            counter &= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Or]): {
                                                            counter |= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Xor]): {
                                                            counter ^= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect operation with integer variable";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        if (counter > LONG_MAX) {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Size overflow";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                        nasm_vars += "\t" + words[1] + ": dd " + std::to_string(counter)  + "\n";
                                        Variables.Add_value(words[1], std::to_string(counter), 'd');
                                        return true;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too few arguments for variable creation";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[Int64_t]): {
                                if (words.size() == 4) {
                                    if (IsString(words[1]) && words[2] == "=" && IsDigit(words[3]) && !Variables.Exists(words[1])) {
                                        if (std::stoull(words[3]) <= ULLONG_MAX) {
                                            nasm_vars += "\t" + words[1] + ": dq " + words[3] + "\n";
                                            Variables.Add_value(words[1], words[3], 'q');
                                            return true;
                                        }
                                        else {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Size overflow";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (words.size() > 4) {
                                    if (IsString(words[1]) && words[2] == "=" && !Variables.Exists(words[1])) {
                                        long long counter = 0;
                                        isfirst = true;
                                        for (size_t j = 3; j < words.size() - 1; ++j) {
                                            if (isfirst) {
                                                if (IsDigit(words[j-1]) && IsDigit(words[j+1]) && (j+1 <= words.size())) {
                                                    switch (HashFunc(words[j].c_str())) {
                                                        case HashFunc(Operator[Addition]): {
                                                            counter = std::stoi(words[j-1]) + std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Subtraction]): {
                                                            counter = std::stoi(words[j-1]) - std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Multiplication]): {
                                                            counter = std::stoi(words[j-1]) * std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Division]): {
                                                            counter = std::stoi(words[j-1]) / std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Remainder]): {
                                                            counter = std::stoi(words[j-1]) % std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[And]): {
                                                            counter = std::stoi(words[j-1]) & std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Or]): {
                                                            counter = std::stoi(words[j-1]) | std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Xor]): {
                                                            counter = std::stoi(words[j-1]) ^ std::stoi(words[j+1]);
                                                            isfirst = false;
                                                            break;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect operation with integer variable";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false;
                                                        }
                                                    }
                                                }
                                            }
                                            else  {
                                                if (IsDigit(words[j+1]) && (j+1 <= words.size())) {
                                                    switch (HashFunc(words[j].c_str())) {
                                                        case HashFunc(Operator[Addition]): {
                                                            counter += std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Subtraction]): {
                                                            counter -= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Multiplication]): {
                                                            counter *= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Division]): {
                                                            counter /= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Remainder]): {
                                                            counter %= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[And]): {
                                                            counter &= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Or]): {
                                                            counter |= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        case HashFunc(Operator[Xor]): {
                                                            counter ^= std::stoi(words[j+1]);
                                                            break;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect operation with integer variable";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        // Not needed for check here because it will be higher then maximum long long size so that just makes this to lowest long long value instead of catching it
                                        nasm_vars += "\t" + words[1] + ": dq " + std::to_string(counter)  + "\n";
                                        Variables.Add_value(words[1], std::to_string(counter), 'q');
                                        return true;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too few arguments for variable creation";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[String8_t]): { 
                                if (words.size() == 4) {
                                    if (IsString(words[1]) && words[2] == "=" && IsSpecialString(words[3]) && !Variables.Exists(words[1])) {
                                        nasm_vars += "\t" + words[1] + ": db " + words[3] + "\n";
                                        Variables.Add_value(words[1], words[3], 'b');
                                        return true;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (words.size() > 4) {
                                    if (words[words.size()-1] != "\"" || words[3] != "\"") {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    size_t j = 4;
                                    std::string full_word {};
                                    while(words[j] != "\"" && j < words.size()-1) {
                                        full_word += words[j];
                                        if ((j+1) != words.size()-1) {
                                            full_word += ' ';
                                        }
                                        ++j;
                                    }
                                    words[3] = '\"' + full_word + '\"';
                                    words.resize(4);
                                    full_word.assign(full_word.length(), '\0');
                                    full_word.clear();
                                    nasm_vars += "\t" + words[1] + ": db " + words[3] + "\n";
                                    Variables.Add_value(words[1], words[3], 'b');
                                    return true;
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too few arguments for variable creation";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[String16_t]): {
                                if (words.size() == 4) {
                                    if (IsString(words[1]) && words[2] == "=" && IsSpecialString(words[3]) && !Variables.Exists(words[1])) {
                                        nasm_vars += "\t" + words[1] + ": dw " + words[3] + "\n";
                                        Variables.Add_value(words[1], words[3], 'w');
                                        return true;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (words.size() > 4) {
                                    if (words[words.size()-1] != "\"" || words[3] != "\"") {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    size_t j = 4;
                                    std::string full_word {};
                                    while(words[j] != "\"" && j < words.size()-1) {
                                        full_word += words[j];
                                        if ((j+1) != words.size()-1) {
                                            full_word += ' ';
                                        }
                                        ++j;
                                    }
                                    words[3] = '\"' + full_word + '\"';
                                    words.resize(4);
                                    full_word.assign(full_word.length(), '\0');
                                    full_word.clear();
                                    nasm_vars += "\t" + words[1] + ": dw " + words[3] + "\n";
                                    Variables.Add_value(words[1], words[3], 'w');
                                    return true;
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too few arguments for variable creation";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[String32_t]): {
                                if (words.size() == 4) {
                                    if (IsString(words[1]) && words[2] == "=" && IsSpecialString(words[3]) && !Variables.Exists(words[1])) {
                                        nasm_vars += "\t" + words[1] + ": dd " + words[3] + "\n";
                                        Variables.Add_value(words[1], words[3], 'd');
                                        return true;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (words.size() > 4) {
                                    if (words[words.size()-1] != "\"" || words[3] != "\"") {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    size_t j = 4;
                                    std::string full_word {};
                                    while(words[j] != "\"" && j < words.size()-1) {
                                        full_word += words[j];
                                        if ((j+1) != words.size()-1) {
                                            full_word += ' ';
                                        }
                                        ++j;
                                    }
                                    words[3] = '\"' + full_word + '\"';
                                    words.resize(4);
                                    full_word.assign(full_word.length(), '\0');
                                    full_word.clear();
                                    nasm_vars += "\t" + words[1] + ": dd " + words[3] + "\n";
                                    Variables.Add_value(words[1], words[3], 'd');
                                    return true;
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too few arguments for variable creation";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[String64_t]): {
                                if (words.size() == 4) {
                                    if (IsString(words[1]) && words[2] == "=" && IsSpecialString(words[3]) && !Variables.Exists(words[1])) {
                                        nasm_vars += "\t" + words[1] + ": dq " + words[3] + "\n";
                                        Variables.Add_value(words[1], words[3], 'q');
                                        return true;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (words.size() > 4) {
                                    if (words[words.size()-1] != "\"" || words[3] != "\"") {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect variable syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    size_t j = 4;
                                    std::string full_word {};
                                    while(words[j] != "\"" && j < words.size()-1) {
                                        full_word += words[j];
                                        if ((j+1) != words.size()-1) {
                                            full_word += ' ';
                                        }
                                        ++j;
                                    }
                                    words[3] = '\"' + full_word + '\"';
                                    words.resize(4);
                                    full_word.assign(full_word.length(), '\0');
                                    full_word.clear();
                                    nasm_vars += "\t" + words[1] + ": dq " + words[3] + "\n";
                                    Variables.Add_value(words[1], words[3], 'q');
                                    return true;
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too few arguments for variable creation";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[RETURN]): {
                                if (words.size() == 2) {
                                    if (IsDigit(words[1])){
                                        nasm_cmds += "\tmov rax, 60\n";
                                        nasm_cmds += "\tmov rdi, " + words[1] + "\n";
                                        nasm_cmds += "\tsyscall\n";
                                        return true;
                                    }
                                    else if (IsVariable(words[1]) && Variables.Exists(words[1].substr(1, words[1].length()-2))) {
                                        words[1].erase(0, 1); // removes first '('
                                        words[1].erase(words[1].length()-1, 1); // removes second ')'
                                        if (IsDigit(Variables.Get_value(words[1]))) {
                                            nasm_cmds += "\tmov rax, 60\n";
                                            nasm_cmds += "\tmov rdi, [rel " + words[1] + "]\n";
                                            nasm_cmds += "\tsyscall\n";
                                            return true;
                                        }
                                        else {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Only digit variable can be returned";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                    }
                                    else if (IsString(words[1])) {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Return value cannot be a string";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect syntax for return function";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too many/few arguments for return function";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[PRINT]): {
                                if (words.size() == 2) {
                                    if (IsSpecialString(words[1])) {
                                        nasm_vars += "\tL" + std::to_string(t) + ": dq " + words[1] + "\n"; // 't' in this case is pseudo random number 
                                        nasm_cmds += "\tmov rax, 1\n";
                                        nasm_cmds += "\tmov rdi, 1\n";
                                        nasm_cmds += "\tlea rsi, [rel L" + std::to_string(t) + "]\n";
                                        nasm_cmds += "\tmov rdx, " + std::to_string(strlen(words[1].c_str()) - 2) + "\n";
                                        nasm_cmds += "\tsyscall\n";
                                        return true;
                                    }
                                    // was '(str)' after substr become 'str'
                                    else if (IsVariable(words[1]) && Variables.Exists(words[1].substr(1, (words[1].length())-2))) {
                                        words[1].erase(0, 1); // removes first '('
                                        words[1].erase(words[1].length()-1, 1); // removes second ')'
                                        if (IsDigit(Variables.Get_value(words[1]))) {
                                            std::string temp = Variables.Get_value(words[1]);
                                            nasm_arrs += "\tA" + std::to_string(t) + ": resb " + std::to_string(strlen(temp.c_str())) + "\n"; 
                                            size_t arr_index = 0;
                                            for (const char &c : temp) {
                                                nasm_cmds += "\tmov rax, " + std::to_string(c - '0') + "\n";
                                                nasm_cmds += "\tadd al, \'0\'\n";
                                                nasm_cmds += "\tmov [A" + std::to_string(t) + " + "  + std::to_string(arr_index) + "], al\n";
                                                ++arr_index;
                                            }
                                            nasm_cmds += "\tmov rax, 1\n";
                                            nasm_cmds += "\tmov rdi, 1\n";
                                            nasm_cmds += "\tlea rsi, [rel A" + std::to_string(t) + "]\n";
                                            nasm_cmds += "\tmov rdx, " + std::to_string(arr_index) + "\n";
                                            nasm_cmds += "\tsyscall\n";
                                            return true;
                                        }
                                        else if (IsSpecialString(Variables.Get_value(words[1]))) {
                                            nasm_cmds += "\tmov rax, 1\n";
                                            nasm_cmds += "\tmov rdi, 1\n";
                                            nasm_cmds += "\tlea rsi, [rel " + words[1] + "]\n";
                                            nasm_cmds += "\tmov rdx, " + std::to_string(strlen(Variables.Get_value(words[1]).c_str())-2) + "\n"; // -2 after strlen because without this it will count "" symbols too 
                                            nasm_cmds += "\tsyscall\n";
                                            return true;
                                        }
                                        else {
                                            ManualReport.ErrorID = 0;
                                            ManualReport.Message = "Undefined behaviour";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Print statement accepts only strings or existing variables";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (words.size() > 2) {
                                    if (words[words.size()-1] != "\"" || words[1] != "\"") {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect print function syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    size_t j = 2;
                                    std::string full_word {};
                                    while(words[j] != "\"" && j < words.size()-1) {
                                        full_word += words[j];
                                        if ((j+1) != words.size()-1) {
                                            full_word += ' ';
                                        }
                                        ++j;
                                    }
                                    words[1] = '\"' + full_word + '\"';
                                    words.resize(2);
                                    full_word.assign(full_word.length(), '\0');
                                    full_word.clear();
                                    
                                    nasm_vars += "\tL" + std::to_string(t) + ": dq " + words[1] + "\n";
                                    nasm_cmds += "\tmov rax, 1\n";
                                    nasm_cmds += "\tmov rdi, 1\n";
                                    nasm_cmds += "\tlea rsi, [rel L" + std::to_string(t) + "]\n";
                                    nasm_cmds += "\tmov rdx, " + std::to_string(strlen(words[1].c_str()) - 2) + "\n";
                                    nasm_cmds += "\tsyscall\n";
                                    return true;


                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too few arguments for print function";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[PRINTLN]): {
                                if (words.size() == 2) {
                                    if (IsSpecialString(words[1])) {
                                        nasm_vars += "\tL" + std::to_string(t) + ": db " + words[1] + ", 10\n"; // 't' in this case is pseudo random number
                                        nasm_cmds += "\tmov rax, 1\n";
                                        nasm_cmds += "\tmov rdi, 1\n";
                                        nasm_cmds += "\tlea rsi, [rel L" + std::to_string(t) + "]\n";
                                        nasm_cmds += "\tmov rdx, " + std::to_string(strlen(words[1].c_str()) - 1) + "\n";
                                        nasm_cmds += "\tsyscall\n";
                                        return true;
                                    }
                                    // was '(str)' after substr become 'str'
                                    else if (IsVariable(words[1]) && Variables.Exists(words[1].substr(1, words[1].length()-2))) {
                                        words[1].erase(0, 1); // removes first '('
                                        words[1].erase(words[1].length()-1, 1); // removes second ')'
                                        if (IsDigit(Variables.Get_value(words[1]))) {
                                            std::string temp = Variables.Get_value(words[1]);
                                            nasm_arrs += "\tA" + std::to_string(t) + ": resb " + std::to_string(strlen(temp.c_str()) + 1) + "\n"; // +1 cuz of newline 
                                            size_t arr_index = 0;
                                            for (const char &c : temp) {
                                                nasm_cmds += "\tmov rax, " + std::to_string(c - '0') + "\n";
                                                nasm_cmds += "\tadd al, \'0\'\n";
                                                nasm_cmds += "\tmov [rel A" + std::to_string(t) + " + "  + std::to_string(arr_index) + "], al\n";
                                                ++arr_index;
                                            }
                                            nasm_cmds += "\tmov rax, 10\n";
                                            nasm_cmds += "\tmov [rel A" + std::to_string(t) + " + " + std::to_string(arr_index) + "], rax\n";
                                            nasm_cmds += "\tmov rax, 1\n";
                                            nasm_cmds += "\tmov rdi, 1\n";
                                            nasm_cmds += "\tlea rsi, [rel A" + std::to_string(t) + "]\n";
                                            nasm_cmds += "\tmov rdx, " + std::to_string(arr_index + 1) + "\n"; // +1 cuz of newline char
                                            nasm_cmds += "\tsyscall\n";
                                            return true;
                                        }
                                        else if (IsSpecialString(Variables.Get_value(words[1]))) {
                                            nasm_cmds += "\tmov rax, 1\n";
                                            nasm_cmds += "\tmov rdi, 1\n";
                                            nasm_cmds += "\tlea rsi, [rel " + words[1] + "]\n";
                                            nasm_cmds += "\tmov rdx, " + std::to_string(strlen(Variables.Get_value(words[1]).c_str())-2) + "\n"; 
                                            nasm_cmds += "\tsyscall\n";
                                            nasm_cmds += "\tpush 0x0A\n";
                                            nasm_cmds += "\tmov rax, 1\n";
                                            nasm_cmds += "\tmov rdi, 1\n";
                                            nasm_cmds += "\tlea rsi, [rsp]\n";
                                            nasm_cmds += "\tmov rdx, 1\n";
                                            nasm_cmds += "\tsyscall\n";
                                            nasm_cmds += "\tpop rax\n"; // clean stack
                                            nasm_cmds += "\txor rax, rax\n";
                                            return true;
                                        }
                                        else {
                                            ManualReport.ErrorID = 0;
                                            ManualReport.Message = "Undefined behaviour";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Println statement accepts only strings and existing variables";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else if (words.size() > 2) {
                                    if (words[words.size()-1] != "\"" || words[1] != "\"") {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect println function syntax";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    size_t j = 2;
                                    std::string full_word {};
                                    while(words[j] != "\"" && j < words.size()-1) {
                                        full_word += words[j];
                                        if ((j+1) != words.size()-1) {
                                            full_word += ' ';
                                        }
                                        ++j;
                                    }
                                    words[1] = '\"' + full_word + '\"';
                                    words.resize(2);
                                    full_word.assign(full_word.length(), '\0');
                                    full_word.clear();
                                    nasm_vars += "\tL" + std::to_string(t) + ": db " + words[1] + ", 10\n";
                                    nasm_cmds += "\tmov rax, 1\n";
                                    nasm_cmds += "\tmov rdi, 1\n";
                                    nasm_cmds += "\tlea rsi, [rel L" + std::to_string(t) + "]\n";
                                    nasm_cmds += "\tmov rdx, " + std::to_string(strlen(words[1].c_str()) - 1) + "\n"; // -1 cauze of -2("" characters) + 1(newline character) = -1 
                                    nasm_cmds += "\tsyscall\n";
                                    return true;

                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too few arguments for println function";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            // TODO 
                            case HashFunc(Commands[READ]): {
                                if (words.size() == 3) {
                                    if (words[1].front() == '(' && words[1].back() == ',' && words[2].back() == ')') {
                                        words[1].erase(0, 1); // removes '('
                                        words[1].erase(words[1].length()-1, 1); // removes ','
                                        words[2].erase(words[2].length()-1, 1); // removes ')'
                                        if (words[1].empty() || words[2].empty()) {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Arguments don't exists";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                        if (Variables.Exists(words[1]) && IsDigit(words[2])) {
                                            if (IsSpecialString(Variables.Get_value(words[1]))) {
                                                std::string temp = "\"";
                                                temp.append(std::stoul(words[2]), '@');
                                                temp += '\"';
                                                nasm_arrs += "\tA" + std::to_string(t) + ": resb " + words[2] + "\n";
                                                nasm_cmds += "\tmov rax, 0\n";
                                                nasm_cmds += "\tmov rdi, 0\n";
                                                nasm_cmds += "\tlea rsi, [rel A" + std::to_string(t) + "]\n";
                                                nasm_cmds += "\tmov rdx, " + words[2] + "\n";
                                                nasm_cmds += "\tsyscall\n";
                                                nasm_cmds += "\tmov rax, [rel A" + std::to_string(t) + "]\n";
                                                nasm_cmds += "\tmov [rel " + words[1] + "], rax\n";
                                                Variables.Change_value(words[1], temp);
                                                return true;
                                            }
                                            else {
                                                ManualReport.ErrorID = 1;
                                                ManualReport.Message = "Only integer variables can be passed";
                                                ManualReport.PrintError(ManualReport);
                                                return false;
                                            }
                                        }
                                        else {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Read function accepts only string variable where input will be stored and maximum size of the input";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect syntax for read function";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too many/few arguments for read function";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[GOTO]): {
                                if (words.size() == 2) {
                                    if (words[1] == "start") {
                                        nasm_cmds += "\tjmp _start\n";
                                        return true;
                                    }
                                    else if (Functions.Called(words[1])) {
                                        nasm_cmds += "\tjmp " + words[1] + "\n";
                                        return true;
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Provided function don't exists";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Too many/few arguments for goto function";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }
                            case HashFunc(Commands[GOTO_IF]): { 
                                if (words.size() == 5) {
                                    if (words[1].front() == '(' && words[3].back() == ',' && words[4].back() == ')') {
                                        // was 'func)' after substr become 'func'
                                        if (Functions.Called(words[4].substr(0, words[4].length()-1)) || words[4].substr(0, words[4].length()-1) == "start") {
                                            words[1].erase(0, 1); // removes '('
                                            words[3].erase(words[3].length()-1, 1); // removes ','
                                            words[4].erase(words[4].length()-1, 1); // removes ')'
                                            if (words[1].empty() || words[3].empty() || words[4].empty()) {
                                                ManualReport.ErrorID = 1;
                                                ManualReport.Message = "Too few arguments for goto_if function";
                                                ManualReport.PrintError(ManualReport);
                                                return false;
                                            }
                                            if (Variables.Exists(words[1]) || Variables.Exists(words[3])) {
                                                if (IsDigit(words[1]) && Variables.Exists(words[3])) {
                                                    if (IsString(Variables.Get_value(words[3]))) {
                                                        goto Error;
                                                    }
                                                    switch (HashFunc(words[2].c_str())) {
                                                        case HashFunc(CompareSymbol[EQ]): {
                                                            switch (Variables.Get_size(words[3])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[NEQ]): {
                                                            switch (Variables.Get_size(words[3])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[L]): {
                                                            switch (Variables.Get_size(words[3])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[LE]): {
                                                            switch (Variables.Get_size(words[3])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[G]): {
                                                            switch (Variables.Get_size(words[3])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[GE]): {
                                                            switch (Variables.Get_size(words[3])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, " + words[1] + "\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect comparison operator";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false; // this works btw
                                                        }
                                                    }
                                                }
                                                else if (IsDigit(words[3]) && Variables.Exists(words[1])) {
                                                    if (IsString(Variables.Get_value(words[1]))) {
                                                        goto Error;
                                                    }
                                                    switch (HashFunc(words[2].c_str())) {
                                                        case HashFunc(CompareSymbol[EQ]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[NEQ]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[L]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[LE]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[G]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[GE]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, " + words[3] + "\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect comparison operator";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false; // this works btw
                                                        }
                                                    }
                                                }
                                                else if (Variables.Exists(words[1]) && Variables.Exists(words[3])) {
                                                    if (IsString(Variables.Get_value(words[3])) || IsString(Variables.Get_value(words[1]))) {
                                                        goto Error;
                                                    }
                                                    else if (Variables.Get_size(words[1]) != Variables.Get_size(words[3])) {
                                                        ManualReport.ErrorID = 1;
                                                        ManualReport.Message = "Cannot compare variables with different sizes";
                                                        ManualReport.PrintError(ManualReport);
                                                        return false;
                                                    }
                                                    else if (words[1] == words[3]) {
                                                        ManualReport.ErrorID = 1;
                                                        ManualReport.Message = "Cannot compare identical variables";
                                                        ManualReport.PrintError(ManualReport);
                                                        return false;
                                                    }
                                                    switch (HashFunc(words[2].c_str())) {
                                                        case HashFunc(CompareSymbol[EQ]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tje " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[NEQ]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjne " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[L]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjl " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[LE]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjle " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[G]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjg " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        case HashFunc(CompareSymbol[GE]): {
                                                            switch (Variables.Get_size(words[1])) {
                                                                case 'b': {
                                                                    nasm_cmds += "\tmov al, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bl, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp al, bl\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'w': {
                                                                    nasm_cmds += "\tmov ax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov bx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp ax, bx\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                case 'd': {
                                                                    nasm_cmds += "\tmov eax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov ebx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp eax, ebx\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                                default: {
                                                                    nasm_cmds += "\tmov rax, [rel " + words[1] + "]\n";
                                                                    nasm_cmds += "\tmov rbx, [rel " + words[3] + "]\n";
                                                                    nasm_cmds += "\tcmp rax, rbx\n";
                                                                    nasm_cmds += "\tjge " + (words[4] == "start" ? "_start" : words[4]) + "\n";
                                                                    break;
                                                                }
                                                            }
                                                            return true;
                                                        }
                                                        default: {
                                                            ManualReport.ErrorID = 1;
                                                            ManualReport.Message = "Incorrect comparison operator";
                                                            ManualReport.PrintError(ManualReport);
                                                            return false; // this works btw
                                                        }
                                                    }
                                                }
                                                Error: {
                                                    ManualReport.ErrorID = 1;
                                                    ManualReport.Message = "Cannot compare variables with strings yet";
                                                    ManualReport.PrintError(ManualReport);
                                                    return false;
                                                }
                                            }
                                            else {
                                                ManualReport.ErrorID = 1;
                                                ManualReport.Message = "You cannot compare with only digits or strings";
                                                ManualReport.PrintError(ManualReport);
                                                return false;
                                            }
                                        }
                                        else {
                                            ManualReport.ErrorID = 1;
                                            ManualReport.Message = "Provided function don't exists";
                                            ManualReport.PrintError(ManualReport);
                                            return false;
                                        }
                                    }
                                    else {
                                        ManualReport.ErrorID = 1;
                                        ManualReport.Message = "Incorrect syntax for goto_if function";
                                        ManualReport.PrintError(ManualReport);
                                        return false;
                                    }
                                    
                                }
                                else {
                                    ManualReport.ErrorID = 1;
                                    ManualReport.Message = "Incorrect placement of arguments for goto_if function";
                                    ManualReport.PrintError(ManualReport);
                                    return false;
                                }
                                break;
                            }

                        }
                        return true; // needed in case of break will happen
                    }
                }
                return true; // needed so compiler won't complain about that no value returned
            };
            if (check_loop() == false) {
                return false;
            }
            sentence.assign(sentence.size(), '\0');
            sentence.clear();
            words.assign(words.size(), "\0"); // fill with invisible values(needed so .clear() will not leave values in memory of array)
            words.clear(); // set size to 0
            words.shrink_to_fit(); // set capacity to 0


        }
        else {
            sentence += buffer[t];
        }

    }
    return true;
}
std::string Read(const std::string &&input) {
    std::ifstream input_f(input, std::ios::in);
    if (!input_f) {
        ManualReport.ErrorID = 0;
        ManualReport.Message = "Don't have permission to read file, or file don't exists";
        ManualReport.PrintError(ManualReport);    
        return "";
    }

    std::ifstream f_length(input, std::ios::binary | std::ios::ate);
    if (f_length.tellg() == 0) {
         ManualReport.ErrorID = 0;
         ManualReport.Message = "File cannot be empty";
         ManualReport.PrintError(ManualReport);
         return "";
    }
    else if (f_length.tellg() < 0) { // tellg returns long int so < 0 will be if size over LONG_MAX macro
        ManualReport.ErrorID = 0;
        ManualReport.Message = "File size too big";
        ManualReport.PrintError(ManualReport);
        return "";
    }
    std::unique_ptr <char[]> buffer = std::make_unique<char[]>((static_cast<size_t>(f_length.tellg())) + 1);
    f_length.close();

    char character;
    unsigned int i = 0;
    for (; input_f.get(character); ++i) {
        buffer[i] = character;
    }
    input_f.close();
    buffer[i] = '\0';
    if (FindStart(buffer) == false) {
        return "";
    }
    std::string nasm_vars = "section .data\n";
    std::string nasm_arrs = "section .bss\n";
    std::string nasm_cmds = "section .text\n\tglobal _start\n_start:\n";
    std::string nasm_buffer {};
    if (Tokenize(buffer, nasm_vars, nasm_arrs, nasm_cmds)) {
        nasm_buffer = nasm_vars + nasm_arrs + nasm_cmds;
    }
    return nasm_buffer;
}
