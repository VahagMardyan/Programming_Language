#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <stack>
#include "../Compiler/compiler.h"
#include "../SymbolTable/symbol_table.h"
#include "../Lexer/lexer.h"
#include "../Parser/parser.h"

class VirtualMachine {
private:
    std::array<Value, 256> regs;   // x0..x31
    std::vector<Value> memory;    // addressable memory
    std::vector<Instruction> prog;
    std::vector<double> consts;
    std::vector<std::string> strings;
    size_t pc;
    int32_t sp;   // stack pointer
    int32_t fp;   // frame pointer
    bool debug_mode;

public:
    VirtualMachine(bool dm = false) : debug_mode(dm) {
        regs.fill(0.0);
        regs[0] = 0.0;
        pc = 0;
        sp = 10000;
        fp = 0;
        memory.resize(20000, 0.0);
    }

    void load(const std::string& expr, SymbolTable& st);
    double run();
    void visualize() const;
};