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

struct CallFrame {
    size_t returnAddress;
    size_t baseReg;
    int argCount;
};

class VirtualMachine {
<<<<<<< HEAD
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
=======
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
>>>>>>> 37c62253fa08934c2bae054db3a95e11c543af6e
};