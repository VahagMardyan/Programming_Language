#pragma once
#include <vector>
#include <string>
#include <memory>
#include "../Compiler/compiler.h"
#include "../Parser/parser.h"
#include "callframe.h"

class Debugger; // forward declaration

class VirtualMachine {
public:
    VirtualMachine(bool debugMode = false);
    ~VirtualMachine();

    void load(const ByteCode& bc);
    void loadFromFile(const std::string& byteCodePath);
    double run();

    // Getters for debugger
    const std::vector<Instruction>& getProgram() const { return current_program; }
    const std::vector<Value>& getRegisters() const { return registers; }
    const std::vector<Value>& getMemory() const { return memory; }
    const std::vector<int>& getLineNumbers() const { return current_lineNumbers; }
    const std::vector<CallFrame>& getCallStack() const { return callStack; }
    const Instruction& getInstructionAt(size_t pc) const { return current_program[pc]; }
    const std::vector<double>& getConstants() const { return current_constants; }
    const std::vector<std::string>& getStrings() const { return current_strings; }

    size_t getPc() const { return pc; }

    // Setters for debugger
    void setPc(size_t newPc) { pc = newPc; }
    void setContinueFlag(bool cont) { debug_continue = cont; }
    bool getContinueFlag() const { return debug_continue; }

    // Core execution: execute one instruction and return new pc
    size_t executeSingleInstruction();
    
private:
    void loadByteCode(const ByteCode& bc);

    // Core state
    std::vector<Value> registers;
    std::vector<Value> memory;
    std::vector<Instruction> current_program;
    std::vector<double> current_constants;
    std::vector<std::string> current_strings;
    std::vector<int> current_lineNumbers;
    std::vector<CallFrame> callStack;
    std::vector<Value> argBuffer;

    size_t pc = 0;
    int lastDestReg = 0;
    bool debug_mode = false;
    bool debug_continue = true; // when false, single-step

    // Globals metadata
    size_t vmGlobalSlotCount = 0;
    std::vector<std::string> vmGlobalNames;
    std::vector<bool> globalDefined;

    std::unique_ptr<Debugger> debugger;
};