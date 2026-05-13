#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <cstdint>
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
    size_t returnDest;
    Value callerSp;
    Value callerFp;
    std::vector<Value> args;
    std::vector<Value> callerRegisters;
};

class VirtualMachine {
    private:
        std::vector<Value> registers;
        std::vector<Value> memory;
        std::vector<std::string> current_strings;
        std::vector<Instruction> current_program;
        std::vector<double> current_consants;
        bool debug_mode;
        void visualize(const std::vector<Instruction>& program) const;
        void loadByteCode(const ByteCode& bc);
        std::vector<CallFrame> callStack;
        std::vector<Value> argBuffer;
        std::vector<int> current_lineNumbers;

        size_t vmGlobalSlotCount = 0;
        std::vector<std::string> vmGlobalNames;
        std::vector<bool> globalDefined;

        // // Debugger
        bool debug_step_mode = false;
        bool debug_continue = false;

        void debugPrompt(size_t pc, const Instruction& inst);
        void printInstructionCompact(size_t pc, const Instruction& inst) const;
    public:
        VirtualMachine(bool dm = false) : debug_mode(dm) {
            registers.resize(256, 0.0);
            memory.resize(20000, 0.0);
        }
        void load(const std::string& expr, SymbolTable& st);
        void load(const ByteCode& bc);
        void loadFromFile(const std::string& byteCodePath);
        double run();
};