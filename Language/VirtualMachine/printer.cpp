#pragma once
#include "debugger.h"
#include "vm.h"
#include "../Compiler/compiler.h"
#include <iostream>
#include <iomanip>

void Debugger::printInstructionCompact(size_t pc) const {
    if (pc >= vm.getProgram().size()) return;
    const Instruction& inst = vm.getProgram()[pc];
    OpCode op = static_cast<OpCode>(inst.op);
    std::cout << "[" << pc << "] ";
    std::cout << std::left;
    
    switch (op) {
        case OpCode::ADD:         std::cout << "ADD r" << inst.dst << " = r" << inst.left << " + r" << inst.right; break;
        case OpCode::MOV:         std::cout << "MOV r" << inst.dst << " = r" << inst.left; break;
        case OpCode::SUB:         std::cout << "SUB r" << inst.dst << " = r" << inst.left << " - r" << inst.right; break;
        case OpCode::AND:         std::cout << "AND r" << inst.dst << " = r" << inst.left << " & r" << inst.right; break;
        case OpCode::OR:          std::cout << "OR r" << inst.dst << " = r" << inst.left << " | r" << inst.right; break;
        case OpCode::XOR:         std::cout << "XOR r" << inst.dst << " = r" << inst.left << " ^ r" << inst.right; break;
        case OpCode::NOT:         std::cout << "NOT r" << inst.dst << " = ~r" << inst.left; break;
        case OpCode::SLL:         std::cout << "SLL r" << inst.dst << " = r" << inst.left << " << r" << inst.right; break;
        case OpCode::SRL:         std::cout << "SRL r" << inst.dst << " = r" << inst.left << " >> r" << inst.right; break;
        case OpCode::SRA:         std::cout << "SRA r" << inst.dst << " = r" << inst.left << " >>> r" << inst.right; break;
        case OpCode::SLT:         std::cout << "SLT r" << inst.dst << " = r" << inst.left << " < r" << inst.right; break;
        case OpCode::SLTU:        std::cout << "SLTU r" << inst.dst << " = r" << inst.left << " < r" << inst.right; break;
        
        case OpCode::MUL:         std::cout << "MUL r" << inst.dst << " = r" << inst.left << " * r" << inst.right; break;
        case OpCode::DIV:         std::cout << "DIV r" << inst.dst << " = r" << inst.left << " / r" << inst.right; break;
        case OpCode::MODULO:      std::cout << "MOD r" << inst.dst << " = r" << inst.left << " % r" << inst.right; break;
        case OpCode::POW:         std::cout << "POW r" << inst.dst << " = r" << inst.left << " ** r" << inst.right; break;
        case OpCode::FLOOR_DIV:   std::cout << "FLOOR_DIV r" << inst.dst << " = r" << inst.left << " // r" << inst.right; break;
        case OpCode::FRAC_DIV:    std::cout << "FRAC_DIV r" << inst.dst << " = frac(r" << inst.left << " / r" << inst.right << ")"; break;
        case OpCode::UNARY:       std::cout << "NEG r" << inst.dst << " = -r" << inst.left; break;
        
        case OpCode::LOAD_CONST:  std::cout << "LOAD_CONST r" << inst.dst << " = " << vm.getConstants()[inst.left]; break;
                case OpCode::LOAD_VAR:    std::cout << "LOAD_VAR r" << inst.dst << " = mem[" << inst.left << "]"; break;
                case OpCode::LOAD_OUTER:  std::cout << "LOAD_OUTER r" << inst.dst << " hops=" << inst.left << " off=" << (int)(int8_t)inst.right; break;
                case OpCode::LOAD_STR:    std::cout << "LOAD_STR r" << inst.dst << " = \"" << vm.getStrings()[inst.left] << "\""; break;
        case OpCode::LOAD_NONE:   std::cout << "LOAD_NONE r" << inst.dst; break;
        case OpCode::LOAD:        std::cout << "LOAD r" << inst.dst << " = [fp" << ((int8_t)inst.right >= 0 ? "+" : "") << (int)(int8_t)inst.right << "]"; break;
        case OpCode::STORE:       std::cout << "STORE r" << inst.dst << " -> fp" << ((int8_t)inst.right >= 0 ? "+" : "") << (int)(int8_t)inst.right; break;
                case OpCode::STORE_VAR:   std::cout << "STORE_VAR r" << inst.right << " -> mem[" << inst.left << "]"; break;
                case OpCode::STORE_OUTER: std::cout << "STORE_OUTER r" << inst.dst << " hops=" << inst.left << " off=" << (int)(int8_t)inst.right; break;
        
        case OpCode::CMP_GT:      std::cout << "CMP_GT r" << inst.dst << " = r" << inst.left << " > r" << inst.right; break;
        case OpCode::CMP_LT:      std::cout << "CMP_LT r" << inst.dst << " = r" << inst.left << " < r" << inst.right; break;
        case OpCode::CMP_GET:     std::cout << "CMP_GET r" << inst.dst << " = r" << inst.left << " >= r" << inst.right; break;
        case OpCode::CMP_LET:     std::cout << "CMP_LET r" << inst.dst << " = r" << inst.left << " <= r" << inst.right; break;
        case OpCode::CMP_EQ:      std::cout << "CMP_EQ r" << inst.dst << " = r" << inst.left << " == r" << inst.right; break;
        case OpCode::CMP_NEQ:     std::cout << "CMP_NEQ r" << inst.dst << " = r" << inst.left << " != r" << inst.right; break;
        
        case OpCode::JMP:         std::cout << "JMP " << getAddress(inst); break;
        case OpCode::JZ:          std::cout << "JZ r" << inst.dst << " -> " << getAddress(inst); break;
        case OpCode::JNZ:         std::cout << "JNZ r" << inst.dst << " -> " << getAddress(inst); break;
        
        case OpCode::PRINT:       std::cout << "PRINT r" << inst.dst; break;
        case OpCode::PRINT_STR:   std::cout << "PRINT_STR \"" << vm.getStrings()[inst.dst] << "\""; break;
        
        case OpCode::LOGICAL_AND: std::cout << "LOG_AND r" << inst.dst << " = r" << inst.left << " && r" << inst.right; break;
        case OpCode::LOGICAL_OR:  std::cout << "LOG_OR r" << inst.dst << " = r" << inst.left << " || r" << inst.right; break;
        case OpCode::LOGICAL_NOT: std::cout << "LOG_NOT r" << inst.dst << " = !r" << inst.left; break;
        
        case OpCode::CALL:        std::cout << "CALL r" << inst.dst << " -> " << getAddress(inst); break;
        case OpCode::RETURN:      std::cout << "RETURN r" << inst.dst; break;
        case OpCode::PUSH_ARG:    std::cout << "PUSH_ARG r" << inst.dst; break;
        case OpCode::LOAD_PARAM:  std::cout << "LOAD_PARAM r" << inst.dst << " = arg[" << inst.left << "]"; break;
        
        case OpCode::INPUT:       std::cout << "INPUT r" << inst.dst; break;
        case OpCode::LENGTH:      std::cout << "LENGTH r" << inst.dst << " = len(r" << inst.left << ")"; break;
        case OpCode::LOAD_STR_IDX: std::cout << "LOAD_STR_IDX r" << inst.dst << " = r" << inst.left << "[r" << inst.right << "]"; break;
        case OpCode::STORE_STR_IDX: std::cout << "STORE_STR_IDX r" << inst.left << "[r" << inst.right << "] = r" << inst.dst; break;
        
        case OpCode::SIN:         std::cout << "SIN r" << inst.dst << " = sin(r" << inst.left << ")"; break;
        case OpCode::COS:         std::cout << "COS r" << inst.dst << " = cos(r" << inst.left << ")"; break;
        case OpCode::TAN:         std::cout << "TAN r" << inst.dst << " = tan(r" << inst.left << ")"; break;
        case OpCode::ASIN:        std::cout << "ASIN r" << inst.dst << " = asin(r" << inst.left << ")"; break;
        case OpCode::ACOS:        std::cout << "ACOS r" << inst.dst << " = acos(r" << inst.left << ")"; break;
        case OpCode::ATAN:        std::cout << "ATAN r" << inst.dst << " = atan(r" << inst.left << ")"; break;
        case OpCode::ATAN2:       std::cout << "ATAN2 r" << inst.dst << " = atan2(r" << inst.left << ", r" << inst.right << ")"; break;
        case OpCode::SQRT:        std::cout << "SQRT r" << inst.dst << " = sqrt(r" << inst.left << ")"; break;
        case OpCode::EXP:         std::cout << "EXP r" << inst.dst << " = exp(r" << inst.left << ")"; break;
        case OpCode::LOG:         std::cout << "LOG r" << inst.dst << " = log(r" << inst.left << ")"; break;
        case OpCode::LOG10:       std::cout << "LOG10 r" << inst.dst << " = log10(r" << inst.left << ")"; break;
        case OpCode::LOG2:        std::cout << "LOG2 r" << inst.dst << " = log2(r" << inst.left << ")"; break;
        case OpCode::CEIL:        std::cout << "CEIL r" << inst.dst << " = ceil(r" << inst.left << ")"; break;
        case OpCode::FLOOR:       std::cout << "FLOOR r" << inst.dst << " = floor(r" << inst.left << ")"; break;
        case OpCode::ABS:         std::cout << "ABS r" << inst.dst << " = abs(r" << inst.left << ")"; break;
        case OpCode::ROUND:       std::cout << "ROUND r" << inst.dst << " = round(r" << inst.left << ")"; break;
        case OpCode::FMOD:        std::cout << "FMOD r" << inst.dst << " = fmod(r" << inst.left << ", r" << inst.right << ")"; break;
        case OpCode::CBRT:        std::cout << "CBRT r" << inst.dst << " = cbrt(r" << inst.left << ")"; break;
        case OpCode::MATH_POW:    std::cout << "POW r" << inst.dst << " = pow(r" << inst.left << ", r" << inst.right << ")"; break;
        case OpCode::LOG_AB:      std::cout << "LOG_AB r" << inst.dst << " = log(r" << inst.right << ") / log(r" << inst.left << ")"; break;
        
        case OpCode::CONST_PI:    std::cout << "CONST_PI r" << inst.dst; break;
        case OpCode::CONST_E:     std::cout << "CONST_E r" << inst.dst; break;
        case OpCode::CONST_INF:     std::cout << "CONST_INF r" << inst.dst; break;
        case OpCode::CONST_MAX:     std::cout << "CONST_MAX r" << inst.dst; break;
        
        case OpCode::ADDI:        std::cout << "ADDI r" << inst.dst << " = r" << inst.left << " + " << (int32_t)(int8_t)inst.right; break;
        case OpCode::ANDI:        std::cout << "ANDI r" << inst.dst << " = r" << inst.left << " & " << (int32_t)(int8_t)inst.right; break;
        case OpCode::ORI:         std::cout << "ORI r" << inst.dst << " = r" << inst.left << " | " << (int32_t)(int8_t)inst.right; break;
        case OpCode::XORI:        std::cout << "XORI r" << inst.dst << " = r" << inst.left << " ^ " << (int32_t)(int8_t)inst.right; break;
        case OpCode::SLLI:        std::cout << "SLLI r" << inst.dst << " = r" << inst.left << " << " << inst.right; break;
        case OpCode::SRLI:        std::cout << "SRLI r" << inst.dst << " = r" << inst.left << " >> " << inst.right; break;
        case OpCode::SRAI:        std::cout << "SRAI r" << inst.dst << " = r" << inst.left << " >>> " << inst.right; break;
        case OpCode::LW:          std::cout << "LW r" << inst.dst << " = mem[" << inst.left << "]"; break;
        case OpCode::SW:          std::cout << "SW mem[" << inst.left << "] = r" << inst.dst; break;
        
        case OpCode::TYPE:        std::cout << "TYPE r" << inst.dst << " = type(r" << inst.left << ")"; break;
        case OpCode::ORD:         std::cout << "ORD r" << inst.dst << " = ord(r" << inst.left << ")"; break;
        case OpCode::CHR:         std::cout << "CHR r" << inst.dst << " = chr(r" << inst.left << ")"; break;
        case OpCode::BIN:         std::cout << "BIN r" << inst.dst << " = bin(r" << inst.left << ")"; break;
        case OpCode::HEX:         std::cout << "HEX r" << inst.dst << " = hex(r" << inst.left << ")"; break;
        case OpCode::OCT:         std::cout << "OCT r" << inst.dst << " = oct(r" << inst.left << ")"; break;
        case OpCode::DEC:         std::cout << "DEC r" << inst.dst << " = dec(r" << inst.left << ")"; break;

        case OpCode::RANDOM: {
            if(inst.left == 0 && inst.right == 0) {
                std::cout << "RANDOM r" << inst.dst << " = random()";
            } else {
                std::cout << "RANDOM r" << inst.dst << " = random(r"
                          << inst.left << ", r" << inst.right << ")";
            }
        };
        break;

        default:                  std::cout << "OP(" << (int)op << ")"; break;
    }
    std::cout << std::endl;
}

void Debugger::visualize() const {
    std::cout << "\n[VM Bytecode Visualization]" << std::endl;
    std::cout << std::left << std::setw(6)  << "Addr"
              << std::setw(12) << "OpCode" << std::setw(6)  << "L" 
              << std::setw(6)  << "R" << std::setw(6)  << "Dst" << " Value" << std::endl; 
    std::cout << std::string(50, '-') << std::endl;

    for(size_t i = 0; i < vm.getProgram().size(); ++i) {
        const auto& inst = vm.getProgram()[i];
        OpCode op = static_cast<OpCode>(inst.op);

        std::cout << "[" << std::setw(3) << i << "]  ";
        std::cout << std::left << std::setw(12);

        switch(op) {
            case OpCode::LOAD_CONST:
                std::cout << "LOAD_CONST" << std::setw(6) << "-" << std::setw(6) << "-"
                          << std::setw(6) << inst.dst << vm.getConstants()[inst.left];
                break;

            case OpCode::LOAD_VAR:
                std::cout << "LOAD_VAR" << std::setw(6) << inst.left << std::setw(6) << "-"
                          << std::setw(6) << inst.dst;
                break;

            case OpCode::LOAD_STR:
                std::cout << "LOAD_STR" << std::setw(6) << inst.left << std::setw(6) << "-"
                          << std::setw(6) << inst.dst << "\"" << vm.getStrings()[inst.left] << "\"";
                break;
            
            case OpCode::LOAD_NONE:
                std::cout << "LOAD_NONE" << std::setw(6) << inst.dst;
                break;
            case OpCode::STORE_VAR:
                std::cout << "STORE_VAR" << std::setw(6) << inst.left << std::setw(6) << inst.right
                          << std::setw(6) << "-" << " mem[" << inst.left << "] = r" << inst.right;
                break;

            case OpCode::LOAD_OUTER:
                std::cout << "LOAD_OUTER" << std::setw(6) << inst.left << std::setw(6) << (int)(int8_t)inst.right
                          << std::setw(6) << inst.dst;
                break;
            case OpCode::STORE_OUTER:
                std::cout << "STORE_OUTER" << std::setw(6) << inst.left << std::setw(6) << (int)(int8_t)inst.right
                          << std::setw(6) << inst.dst;
                break;

            case OpCode::ADD:     std::cout << "ADD";     break;
            case OpCode::MOV:     std::cout << "MOV";     break;
            case OpCode::SUB:     std::cout << "SUB";     break;
            case OpCode::MUL:     std::cout << "MUL";     break;
            case OpCode::DIV:     std::cout << "DIV";     break;
            case OpCode::FLOOR_DIV: std::cout << "FLOOR_DIV"; break;
            case OpCode::FRAC_DIV: std::cout << "FRAC_DIV"; break;
            case OpCode::POW:     std::cout << "POW";     break;
            case OpCode::MATH_POW: std::cout<<"MATH_POW"; break;
            case OpCode::MODULO:  std::cout << "MODULO";  break;
            case OpCode::UNARY:   std::cout << "NEG";     break;

            case OpCode::NOT:     std::cout << "NOT";     break;
            case OpCode::AND:     std::cout << "AND";     break;
            case OpCode::OR:      std::cout << "OR";      break;
            case OpCode::XOR:     std::cout << "XOR";     break;
            case OpCode::SLL:     std::cout << "SLL";     break;
            case OpCode::SRL:     std::cout << "SRL";     break;
            case OpCode::SRA:     std::cout << "SRA";     break;
            case OpCode::SLT:     std::cout << "SLT";     break;
            case OpCode::SLTU:    std::cout << "SLTU";    break;

            case OpCode::CMP_GT:  std::cout << "CMP_GT";  break;
            case OpCode::CMP_LT:  std::cout << "CMP_LT";  break;
            case OpCode::CMP_GET: std::cout << "CMP_GET"; break;
            case OpCode::CMP_LET: std::cout << "CMP_LET"; break;
            case OpCode::CMP_EQ:  std::cout << "CMP_EQ";  break;
            case OpCode::CMP_NEQ: std::cout << "CMP_NEQ"; break;

            case OpCode::LOGICAL_AND:
                std::cout << "LOGICAL_AND" << std::setw(6) << inst.left 
                          << std::setw(6) << inst.right << std::setw(6) << inst.dst;
                break;
            case OpCode::LOGICAL_OR:
                std::cout << "LOGICAL_OR" << std::setw(6) << inst.left 
                          << std::setw(6) << inst.right << std::setw(6) << inst.dst;
                break;
            case OpCode::LOGICAL_NOT:
                std::cout << "LOGICAL_NOT" << std::setw(6) << inst.left 
                          << std::setw(6) << "-" << std::setw(6) << inst.dst;
                break;

            case OpCode::JZ:
                std::cout << "JZ" << std::setw(6) << inst.dst << " TO ADDR: " << getAddress(inst);
                break;
            case OpCode::JMP:
                std::cout << "JMP" << std::setw(6) << inst.dst << " TO: " << getAddress(inst);
                break;

            case OpCode::PRINT:
                std::cout << "PRINT" << std::setw(6) << inst.dst;
                break;

            case OpCode::PRINT_STR:
                std::cout<<"PRINT_STR" << std::setw(6) << inst.dst;
                break;

            case OpCode::CALL:
                std::cout << "CALL" << std::setw(6) << "-" << std::setw(6) << "-"
                          << std::setw(6) << inst.dst << " ADDR: " << getAddress(inst);
                break;

            case OpCode::LENGTH:
                std::cout << "LENGTH" << std::setw(6) << inst.left << std::setw(6) << "-"
                          << std::setw(6) << inst.dst;
            break;
            case OpCode::SIN:   std::cout << "SIN"; break;
            case OpCode::COS:   std::cout << "COS"; break;
            case OpCode::TAN:   std::cout << "TAN"; break;
            case OpCode::ASIN:  std::cout << "ASIN"; break;
            case OpCode::ACOS:  std::cout << "ACOS"; break;
            case OpCode::ATAN:  std::cout << "ATAN"; break;
            case OpCode::ATAN2: std::cout << "ATAN2"; break;
            case OpCode::SQRT:  std::cout << "SQRT"; break;
            case OpCode::EXP:   std::cout << "EXP"; break;
            case OpCode::LOG:   std::cout << "LOG"; break;
            case OpCode::LOG10: std::cout << "LOG10"; break;
            case OpCode::CEIL:  std::cout << "CEIL"; break;
            case OpCode::FLOOR: std::cout << "FLOOR"; break;
            case OpCode::ABS:   std::cout << "ABS"; break;
            case OpCode::ROUND: std::cout << "ROUND"; break;
            case OpCode::FMOD:  std::cout << "FMOD"; break;

            case OpCode::CONST_PI:
                std::cout << "LOAD_CONST PI" << std::setw(6);
            break;

            case OpCode::CONST_E:
                std::cout << "LOAD_CONST E" << std::setw(6);
            break;

            case OpCode::CONST_INF:
                std::cout << "LOAD_CONST INF" << std::setw(6);
            break;

            case OpCode::CONST_MAX:
                std::cout << "LOAD_CONST MAX" << std::setw(6);
            break;
            case OpCode::RETURN:
                std::cout << "RETURN" << std::setw(6) << inst.dst << std::setw(6) << "-" << std::setw(6) << "-";
                break;

            case OpCode::PUSH_ARG:
                std::cout << "PUSH_ARG" << std::setw(6) << inst.dst << std::setw(6) << "-" << std::setw(6) << "-";
                break;

            case OpCode::LOAD_PARAM:
                std::cout << "LOAD_PARAM" << std::setw(6) << inst.left << std::setw(6) << "-" 
                          << std::setw(6) << inst.dst;
                break;

            case OpCode::LOAD:
                std::cout << "LOAD" << std::setw(6) << inst.left << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dst;
                break;

            case OpCode::STORE:
                std::cout << "STORE" << std::setw(6) << inst.left << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dst;
                break;

            case OpCode::ADDI:
                std::cout << "ADDI" 
                          << std::setw(6) << inst.left 
                          << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dst 
                          << " (imm=" << (int32_t)inst.right << ")";
                break;
            case OpCode::ANDI:
                std::cout << "ANDI" << std::setw(6) << inst.left << std::setw(6) << inst.right
                          << std::setw(6) << inst.dst;
                break;
            case OpCode::ORI:
                std::cout << "ORI" << std::setw(6) << inst.left << std::setw(6) << inst.right
                          << std::setw(6) << inst.dst;
                break;
            case OpCode::XORI:
                std::cout << "XORI" << std::setw(6) << inst.left << std::setw(6) << inst.right
                          << std::setw(6) << inst.dst;
                break;
            case OpCode::SLLI:
                std::cout << "SLLI" << std::setw(6) << inst.left << std::setw(6) << inst.right
                          << std::setw(6) << inst.dst;
                break;
            case OpCode::SRLI:
                std::cout << "SRLI" << std::setw(6) << inst.left << std::setw(6) << inst.right
                          << std::setw(6) << inst.dst;
                break;
            case OpCode::SRAI:
                std::cout << "SRAI" << std::setw(6) << inst.left << std::setw(6) << inst.right
                          << std::setw(6) << inst.dst;
                break;
            case OpCode::LW:
                std::cout << "LW" << std::setw(6) << inst.left << std::setw(6) << "-"
                          << std::setw(6) << inst.dst;
                break;
            case OpCode::SW:
                std::cout << "SW" << std::setw(6) << inst.left << std::setw(6) << "-"
                          << std::setw(6) << inst.dst;
                break;

            case OpCode::TYPE: std::cout<<"TYPE";   break;
            case OpCode::ORD:  std::cout << "ORD r" << inst.dst << " = ord(r" << inst.left << ")"; break;
            case OpCode::CHR:  std::cout << "CHR r" << inst.dst << " = chr(r" << inst.left << ")"; break;
            case OpCode::BIN:  std::cout << "BIN";     break;
            case OpCode::HEX:  std::cout << "HEX";     break;
            case OpCode::OCT:  std::cout << "OCT";     break;
            case OpCode::DEC:  std::cout << "DEC";     break;
            case OpCode::RANDOM: std::cout << "RANDOM"; break;
            case OpCode::LOAD_STR_IDX: std::cout << "LOAD_STR_IDX"; break;
            case OpCode::STORE_STR_IDX: std::cout << "STORE_STR_IDX"; break;
            default:
                std::cout << "UNKNOWN (op=" << (int)op << ")";
                break;
        }
        std::cout << std::endl;
    }
}