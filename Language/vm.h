#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "compiler.h"
#include "symbol_table.h"
#include "lexer.h"
#include "tokenizer.h"
#include "parser.h"

class VirtualMachine {
    private:
        std::vector<double> registers;
        std::vector<Instruction> current_program;
        std::vector<double> current_consants;
        bool debug_mode;
        void visualize(const std::vector<Instruction>& program) const;
    public:
        VirtualMachine(bool dm = false) : debug_mode(dm) {
            registers.resize(256, 0.0);
        }
        void load(const std::string& expr, SymbolTable& st);
        double run(SymbolTable& st);
};