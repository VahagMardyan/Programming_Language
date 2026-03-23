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
        std::vector<Instruction> program;
        std::vector<double> registers;
        int finalIdx = 0;
        bool debug_mode;
        void visualize() const;
    public:
        VirtualMachine(bool dm = false) : debug_mode(dm) {}
        void load(const std::string& expr, SymbolTable& symTable);
        double run(const SymbolTable& symtable); 
};