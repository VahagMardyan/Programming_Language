#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "../Compiler/compiler.h"
#include "../SymbolTable/symbol_table.h"
#include "../Lexer/lexer.h"
#include "../Tokenizer/tokenizer.h"
#include "../Parser/parser.h"

struct CallFrame {
    size_t returnAddress;
    size_t baseReg;
    int argCount;
};

class VirtualMachine {
    private:
        std::vector<Value> registers;
        std::vector<std::string> current_strings;
        std::vector<Instruction> current_program;
        std::vector<double> current_consants;
        bool debug_mode;
        void visualize(const std::vector<Instruction>& program) const;
        std::stack<CallFrame> callStack;
        std::vector<Value> argBuffer;
    public:
        VirtualMachine(bool dm = false) : debug_mode(dm) {
            registers.resize(256, 0.0);
        }
        void load(const std::string& expr, SymbolTable& st);
        double run(SymbolTable& st);
};