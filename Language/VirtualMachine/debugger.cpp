#include "debugger.h"
#include "vm.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <stdexcept>

Debugger::Debugger(VirtualMachine& vm) : vm(vm) {}

void Debugger::reset() {
    breakpoints.clear();
    stepOverDepth = -1;
    stepOutDepth = -1;
    vm.setPc(0);
    vm.setContinueFlag(false); // pause at entry before first instruction
}

void Debugger::run() {
    reset();
    visualize();
    printHelp();

    size_t pc = vm.getPc();
    while (pc < vm.getProgram().size()) {
        while (shouldStop(pc)) {
            printState(pc);
            std::cout << "(dbg) ";
            std::string line;
            if (!std::getline(std::cin, line)) {
                vm.setContinueFlag(true);
                return;
            }
            bool resume = false;
            ParsedCmd cmd = parseCommand(line);
            execCommand(cmd, pc, resume);
            if (resume) break;
            pc = vm.getPc();
        }
        pc = vm.executeSingleInstruction();
        vm.setPc(pc);
    }
}

bool Debugger::shouldStop(size_t currentPc) {
    // Breakpoint hit
    if (breakpoints.count(currentPc)) {
        std::cout << "[dbg] breakpoint hit at " << currentPc << "\n";
        return true;
    }
    // Step-over: call stack depth decreased to <= recorded depth
    if (stepOverDepth >= 0 && (int)vm.getCallStack().size() <= stepOverDepth) {
        stepOverDepth = -1;
        return true;
    }
    // Step-out: call stack depth smaller than recorded depth
    if (stepOutDepth >= 0 && (int)vm.getCallStack().size() < stepOutDepth) {
        stepOutDepth = -1;
        return true;
    }
    // Single-step mode (continue flag false)
    if (!vm.getContinueFlag()) {
        return true;
    }
    return false;
}

void Debugger::execCommand(const ParsedCmd& cmd, size_t pc, bool& resume) {
    switch (cmd.code) {
        case CmdCode::Step:
            stepOverDepth = -1;
            stepOutDepth = -1;
            vm.setContinueFlag(false);
            resume = true;
            break;
        case CmdCode::StepOver:
            stepOverDepth = (int)vm.getCallStack().size();
            vm.setContinueFlag(true);
            resume = true;
            break;
        case CmdCode::StepOut:
            stepOutDepth = (int)vm.getCallStack().size();
            vm.setContinueFlag(true);
            resume = true;
            break;
        case CmdCode::Go:
        case CmdCode::Continue:
            stepOverDepth = -1;
            stepOutDepth = -1;
            vm.setContinueFlag(true);
            resume = true;
            break;
        case CmdCode::Quit:
            throw std::runtime_error("Debug quit by user");
        case CmdCode::BrAdd: {
            size_t addr = 0;
            bool resolved = false;
            if (cmd.args.count("offset")) {
                addr = static_cast<size_t>(std::stoul(cmd.args.at("offset")));
                resolved = true;
            } else if (cmd.args.count("func")) {
                std::cout << "[dbg] -func lookup not yet wired to runtime symbols; use -offset\n";
            } else if (!cmd.positional.empty()) {
                addr = static_cast<size_t>(std::stoul(cmd.positional));
                resolved = true;
            }
            if (resolved) {
                if (addr < vm.getProgram().size()) {
                    breakpoints.insert(addr);
                    std::cout << "[dbg] breakpoint set at " << addr << "\n";
                } else {
                    std::cout << "[dbg] address " << addr << " out of range\n";
                }
            } else {
                std::cout << "[dbg] usage: br.add <addr>  |  br.add -offset <n>\n";
            }
            break;
        }
        case CmdCode::BrRem: {
            size_t addr = 0;
            bool resolved = false;
            if (cmd.args.count("offset")) {
                addr = static_cast<size_t>(std::stoul(cmd.args.at("offset")));
                resolved = true;
            } else if (!cmd.positional.empty()) {
                addr = static_cast<size_t>(std::stoul(cmd.positional));
                resolved = true;
            }
            if (resolved) {
                if (breakpoints.erase(addr))
                    std::cout << "[dbg] breakpoint removed at " << addr << "\n";
                else
                    std::cout << "[dbg] no breakpoint at " << addr << "\n";
            } else {
                std::cout << "[dbg] usage: br.rem <addr>  |  br.rem -offset <n>\n";
            }
            break;
        }
        case CmdCode::BrList:
            printBreakpoints();
            break;
        case CmdCode::PrintReg: {
            std::string s = cmd.positional;
            if (s.empty() && cmd.args.count("n")) s = cmd.args.at("n");
            const auto& regs = vm.getRegisters();
            if (s.empty()) {
                // print all non-zero registers (skip sp, fp)
                for (size_t i = 0; i < regs.size(); ++i) {
                    if (i == 2 || i == 8) continue;
                    if (!isNone(regs[i]) && (isNumber(regs[i]) ? asNumber(regs[i]) != 0.0 : true))
                        std::cout << "r" << i << "=" << valueToString(regs[i]) << " ";
                }
                std::cout << "\n";
            } else {
                int n = std::stoi(s);
                if (n >= 0 && n < (int)regs.size())
                    std::cout << "r" << n << " = " << valueToString(regs[n]) << "\n";
                else
                    std::cout << "[dbg] invalid register r" << n << "\n";
            }
            break;
        }
        case CmdCode::PrintMem: {
            std::string s = cmd.positional;
            if (s.empty() && cmd.args.count("addr")) s = cmd.args.at("addr");
            if (s.empty()) {
                std::cout << "[dbg] usage: m <addr>  |  m -addr <n>\n";
            } else {
                int addr = std::stoi(s);
                const auto& mem = vm.getMemory();
                if (addr >= 0 && addr < (int)mem.size())
                    std::cout << "mem[" << addr << "] = " << valueToString(mem[addr]) << "\n";
                else
                    std::cout << "[dbg] invalid address " << addr << "\n";
            }
            break;
        }
        case CmdCode::Help: {
            printHelp();
        }
        break;
        case CmdCode::Unknown:
            std::cout << "[dbg] unknown command '" << cmd.positional << "'\n";
            break;
        default:
            break;
    }
}

Debugger::ParsedCmd Debugger::parseCommand(const std::string& line) {
    ParsedCmd result;
    std::istringstream iss(line);
    std::string verb;
    iss >> verb;

    if (verb.empty()) result.code = CmdCode::Step;
    else if (verb == "step" || verb == "si") result.code = CmdCode::Step;
    else if (verb == "over" || verb == "so") result.code = CmdCode::StepOver;
    else if (verb == "out"  || verb == "su") result.code = CmdCode::StepOut;
    else if (verb == "go"   || verb == "g")  result.code = CmdCode::Go;
    else if (verb == "c"    || verb == "continue") result.code = CmdCode::Continue;
    else if (verb == "q"    || verb == "quit" || verb == "exit")     result.code = CmdCode::Quit;
    else if (verb == "br.add")  result.code = CmdCode::BrAdd;
    else if (verb == "br.rem")  result.code = CmdCode::BrRem;
    else if (verb == "br" || verb == "br.list") result.code = CmdCode::BrList;
    else if (verb == "r" || verb == "reg")     result.code = CmdCode::PrintReg;
    else if (verb == "m" || verb == "mem")     result.code = CmdCode::PrintMem;
    else if (verb == "help" || verb == "h")    result.code = CmdCode::Help;
    else {
        // shorthand: r3, m100
        if (verb.size() > 1 && verb[0] == 'r' && isdigit(verb[1])) {
            result.code = CmdCode::PrintReg;
            result.positional = verb.substr(1);
            return result;
        }
        if (verb.size() > 1 && verb[0] == 'm' && isdigit(verb[1])) {
            result.code = CmdCode::PrintMem;
            result.positional = verb.substr(1);
            return result;
        }
        result.code = CmdCode::Unknown;
        result.positional = verb;
        return result;
    }

    // parse named args: -key value
    std::string tok;
    bool gotPositional = false;
    while (iss >> tok) {
        if (tok.size() >= 2 && tok[0] == '-' && !isdigit(tok[1])) {
            std::string key = tok.substr(1);
            std::string val;
            if (iss >> val) result.args[key] = val;
            else            result.args[key] = "";
        } else if (!gotPositional) {
            result.positional = tok;
            gotPositional = true;
        }
    }
    return result;
}

void Debugger::printHelp() const {
    std::cout << "\n─── Commands ─────────────────────────────────────────\n"
              << "  Enter / step   Execute next instruction (step into)\n"
              << "  over           Step over function calls\n"
              << "  out            Step out of current function\n"
              << "  go / c         Continue until next breakpoint\n"
              << "  br.add <n>     Set breakpoint at address n\n"
              << "  br.rem <n>     Remove breakpoint at address n\n"
              << "  br             List all breakpoints\n"
              << "  r<n>           Show register value (e.g., r0)\n"
              << "  m<n>           Show memory value (e.g., m100)\n"
              << "  help / h       Show Commands Menu\n"
              << "  quit / q / exit       Quit debugger\n"
              << "────────────────────────────────────────────────────\n";
}

void Debugger::printBreakpoints() const {
    if (breakpoints.empty()) {
        std::cout << "[dbg] no breakpoints set\n";
        return;
    }
    std::cout << "[dbg] breakpoints:";
    for (size_t addr : breakpoints) std::cout << " " << addr;
    std::cout << "\n";
}

void Debugger::printState(size_t pc) {
    std::cout << "\n=== DEBUG @ " << pc;
    const auto& lineNums = vm.getLineNumbers();
    if (pc < lineNums.size() && lineNums[pc] > 0)
        std::cout << "  (line " << lineNums[pc] << ")";
    std::cout << " ===\n";

    std::cout << "  next: ";
    printInstructionCompact(pc);

    const auto& stack = vm.getCallStack();
    if (!stack.empty())
        std::cout << "  call depth: " << stack.size() << "\n";

    // Show non-zero registers (skip sp,fp)
    const auto& regs = vm.getRegisters();
    bool anyReg = false;
    for (size_t i = 0; i < regs.size(); ++i) {
        if (i == 2 || i == 8) continue;
        if (!isNone(regs[i]) && (isNumber(regs[i]) ? asNumber(regs[i]) != 0.0 : true)) {
            if (!anyReg) { std::cout << "  regs: "; anyReg = true; }
            std::cout << "r" << i << "=" << valueToString(regs[i]) << " ";
        }
    }
    if (anyReg) std::cout << "\n";

    // printHelp();
}