#pragma once
#include <set>
#include <map>
#include <string>
#include <cstddef>

class VirtualMachine;

class Debugger {
public:
    explicit Debugger(VirtualMachine& vm);
    void run();                         // main debugging loop
    void reset();                       // clear breakpoints, step state

    // Called by VM's run() in debug mode
    bool shouldStop(size_t currentPc);

private:
    VirtualMachine& vm;

    // Breakpoints
    std::set<size_t> breakpoints;

    // Step control
    int stepOverDepth = -1;
    int stepOutDepth = -1;

    // Command types
    enum class CmdCode {
        Step, StepOver, StepOut, Go, Continue, Quit,
        BrAdd, BrRem, BrList,
        PrintReg, PrintMem,
        Unknown
    };
    struct ParsedCmd {
        CmdCode code = CmdCode::Unknown;
        std::map<std::string, std::string> args;
        std::string positional;
    };

    ParsedCmd parseCommand(const std::string& line);
    void execCommand(const ParsedCmd& cmd, size_t pc, bool& resume);
    void printHelp() const;
    void printBreakpoints() const;
    void printState(size_t pc);

    void printInstructionCompact(size_t pc) const;
    void visualize() const;
};