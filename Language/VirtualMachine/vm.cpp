#include "vm.h"
#include <fstream>
#include <sstream>

// RISC-V register numbers
enum Reg : uint8_t {
    ZERO = 0, RA = 1, SP = 2, GP = 3, TP = 4,
    T0 = 5, T1 = 6, T2 = 7,
    FP = 8, S1 = 9,
    A0 = 10, A1 = 11, A2 = 12, A3 = 13, A4 = 14, A5 = 15, A6 = 16, A7 = 17,
    S2 = 18, S3 = 19, S4 = 20, S5 = 21, S6 = 22, S7 = 23, S8 = 24, S9 = 25, S10 = 26, S11 = 27,
    T3 = 28, T4 = 29, T5 = 30, T6 = 31
};

void VirtualMachine::load(const std::string& expr, SymbolTable& symtable) {
    std::istringstream stream(expr);
    Lexer lexer(stream);
    Tokenizer tokenizer(lexer);
    Parser parser(tokenizer, symtable);
    auto root = std::static_pointer_cast<ASTNode>(parser.parseProgram());
<<<<<<< HEAD
    if(!root) throw std::runtime_error("Parsing failed!");
    Compiler compiler(symtable);
=======
    if (!root) throw std::runtime_error("Parsing failed!");
    Compiler compiler(&symtable);
>>>>>>> 37c62253fa08934c2bae054db3a95e11c543af6e
    ByteCode bc = compiler.compile(root);
    prog = bc.instructions;
    consts = bc.constants;
    strings = bc.strings;

    // Reset state
    regs.fill(0.0);
    regs[ZERO] = 0.0;
    pc = 0;
    sp = 10000;
    fp = 0;
    memory.clear();
    memory.resize(20000, 0.0);

    if (debug_mode) {
        root->print();
        compiler.printByteCode(prog);
    }
}

double VirtualMachine::run() {
    while (pc < prog.size()) {
        Instruction inst = prog[pc];
        OpCode op = static_cast<OpCode>(inst.op);
        bool jumped = false;
<<<<<<< HEAD
        switch(op) {
            case OpCode::LOAD_CONST: registers[inst.dst] = current_consants[inst.left]; break;
            case OpCode::LOAD_VAR:   registers[inst.dst] = st.getValueByAddress(inst.left); break;
            case OpCode::LOAD_STR: registers[inst.dst] = current_strings[inst.left]; break;
            case OpCode::STORE_VAR:  st.setValueByAddress(inst.left, registers[inst.right]); break;
            
            case OpCode::ADD:  {
                if(isString(registers[inst.left]) || isString(registers[inst.right])) {
                    registers[inst.dst] = valueToString(registers[inst.left]) + valueToString(registers[inst.right]);
            } else {
                    registers[inst.dst] = asNumber(registers[inst.left]) + asNumber(registers[inst.right]);
            }
        }
        break;
        case OpCode::SUB:    registers[inst.dst] = asNumber(registers[inst.left]) - asNumber(registers[inst.right]); break;
        case OpCode::MUL:    registers[inst.dst] = asNumber(registers[inst.left]) * asNumber(registers[inst.right]); break;
        case OpCode::DIV:
            if(asNumber(registers[inst.right]) == 0) throw std::runtime_error("Division by zero");
            registers[inst.dst] = asNumber(registers[inst.left]) / asNumber(registers[inst.right]); break;
        case OpCode::UNARY:  registers[inst.dst] = -asNumber(registers[inst.left]); break;
        case OpCode::MODULO: registers[inst.dst] = (double)((long long) asNumber(registers[inst.left]) % (long long) asNumber(registers[inst.right])); break;
        case OpCode::AND:    registers[inst.dst] = (double)((long long) asNumber(registers[inst.left]) & (long long) asNumber(registers[inst.right])); break;
        case OpCode::OR:     registers[inst.dst] = (double)((long long) asNumber(registers[inst.left]) | (long long) asNumber(registers[inst.right])); break;
        case OpCode::XOR:    registers[inst.dst] = (double)((long long) asNumber(registers[inst.left]) ^ (long long) asNumber(registers[inst.right])); break;
        case OpCode::LSHIFT: registers[inst.dst] = (double)((long long) asNumber(registers[inst.left]) << (long long) asNumber(registers[inst.right])); break;
        case OpCode::RSHIFT: registers[inst.dst] = (double)((long long) asNumber(registers[inst.left]) >> (long long) asNumber(registers[inst.right])); break;
        case OpCode::CMP_GT:  registers[inst.dst] = asNumber(registers[inst.left]) >  asNumber(registers[inst.right]) ? 1.0 : 0.0; break;
        case OpCode::CMP_LT:  registers[inst.dst] = asNumber(registers[inst.left]) <  asNumber(registers[inst.right]) ? 1.0 : 0.0; break;
        case OpCode::CMP_GET: registers[inst.dst] = asNumber(registers[inst.left]) >= asNumber(registers[inst.right]) ? 1.0 : 0.0; break;
        case OpCode::CMP_LET: registers[inst.dst] = asNumber(registers[inst.left]) <= asNumber(registers[inst.right]) ? 1.0 : 0.0; break;
        case OpCode::CMP_EQ:  registers[inst.dst] = registers[inst.left] == registers[inst.right] ? 1.0 : 0.0; break;
        case OpCode::CMP_NEQ: registers[inst.dst] = registers[inst.left] != registers[inst.right] ? 1.0 : 0.0; break;
        case OpCode::JZ: {
                if(isFalsy(registers[inst.dst])) {
                    pc = inst.left;
                    jumped = true;
            }
        }
        break;
        case OpCode::JMP: pc = inst.left; jumped = true; break;
        case OpCode::PRINT:
            std::visit([](auto&& val){std::cout<<val;}, registers[inst.dst]);
        break;
        case OpCode::PRINT_STR: std::cout<<current_strings[inst.dst]; break;
        
        case OpCode::LOGICAL_AND: registers[inst.dst] = (isTruthy(registers[inst.left]) && isTruthy(registers[inst.right])) ? 1.0 : 0.0; break;
        case OpCode::LOGICAL_OR: registers[inst.dst] = (isTruthy(registers[inst.left]) || isTruthy(registers[inst.right])) ? 1.0 : 0.0; break;
        case OpCode::LOGICAL_NOT: registers[inst.dst] = (isFalsy(registers[inst.left]) != 0) ? 0.0 : 1.0; break;

        case OpCode::CALL: {
            size_t retAddr = pc + 1;
            uint16_t funcAddr = getAddress(inst);
            size_t baseReg = inst.dst + 1;

            callStack.push({retAddr, baseReg, (int)argBuffer.size()});

            for(int i = 0; i < (int)argBuffer.size(); i++) {
                if(baseReg + i < registers.size())
                    registers[baseReg + i] = argBuffer[i];
            }
            argBuffer.clear();

            pc = funcAddr;
            jumped = true;
        }
        break;

        case OpCode::RETURN: {
                Value retVal = registers[inst.dst];
                if(callStack.empty()) break;
                CallFrame frame = callStack.top();
                callStack.pop();
                registers[frame.baseReg - 1] = retVal;
                pc = frame.returnAddress;
                jumped = true;
        }
        break;
        
        case OpCode::PUSH_ARG:
            argBuffer.push_back(registers[inst.dst]);
        break;
        case OpCode::LOAD_PARAM: {
            if(!callStack.empty()) {
                size_t baseReg = callStack.top().baseReg;
                registers[inst.dst] = registers[baseReg + inst.left];
            }
        }
        break;
        default: break;
    }
    if(!jumped) pc++;
}
if(isNumber(registers[lastDest])) {
        return asNumber(registers[lastDest]) == -0.0 ? 0.0 : asNumber(registers[lastDest]);
}
return 0.0;
}

void VirtualMachine::visualize(const std::vector<Instruction>& program) const {
    std::cout << "\n[VM Bytecode Visualization]" << std::endl;
    std::cout << std::left << std::setw(6)  << "Addr"
              << std::setw(12) << "OpCode" << std::setw(6)  << "L" 
              << std::setw(6)  << "R" << std::setw(6)  << "Dst" << "Value" << std::endl; 
                 std::cout << std::string(45, '-') << std::endl;

    for(size_t i = 0; i < program.size(); ++i) {
        const auto& inst = program[i];
        OpCode op = static_cast<OpCode>(inst.op);
        std::cout << "[" << std::setw(3) << i << "]  ";
        std::cout << std::left << std::setw(12);

        switch(op) {
            case OpCode::LOAD_CONST:
                std::cout << "LOAD_CONST" << std::setw(6) << "-" << std::setw(6) << "-"
                      << std::setw(6) << inst.dst << current_consants[inst.left];
            break;
            case OpCode::LOAD_VAR:
                std::cout << "LOAD_VAR" << std::setw(6) << inst.left << std::setw(6) << "-"
                        << std::setw(6) << inst.dst; 
            break;
            case OpCode::LOAD_STR:
                std::cout << "LOAD_STR" << std::setw(6) << inst.left << std::setw(6) << "-"
                        << std::setw(6) << inst.dst << "\"" << current_strings[inst.left] << "\"";
            break;
            case OpCode::STORE_VAR:
                std::cout << "STORE_VAR" << std::setw(6) << inst.left << std::setw(6) << inst.right
                        << std::setw(6) << "-" << "mem[" << inst.left << "] = r" << inst.right;
            break;
            case OpCode::ADD:
                std::cout << "ADD" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::SUB:
                std::cout << "SUB" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::MUL:
                std::cout << "MUL" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::DIV:
                std::cout << "DIV" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::UNARY:
                std::cout << "NEG" << std::setw(6) << inst.left << std::setw(6) << "-" << std::setw(6) << inst.dst; break;
            case OpCode::AND:
                std::cout << "AND" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::OR:
                std::cout << "OR" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::XOR:
                std::cout << "XOR" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::MODULO:
                std::cout << "MODULO" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::LSHIFT:
                std::cout << "LSHIFT" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::RSHIFT:
                std::cout << "RSHIFT" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::CMP_GT:
                std::cout << "CMP_GT" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::CMP_LT:
                std::cout << "CMP_LT" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::CMP_GET:
                std::cout << "CMP_GET" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::CMP_LET:
                std::cout << "CMP_LET" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::CMP_EQ:
                std::cout << "CMP_EQ" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::CMP_NEQ:
                std::cout << "CMP_NEQ" << std::setw(6) << inst.left << std::setw(6) << inst.right << std::setw(6) << inst.dst; break;
            case OpCode::JZ:
                std::cout << "JZ" << std::setw(6) << inst.dst << "TO ADDR: " << inst.left; break;
            case OpCode::JMP:
                std::cout << "JMP" << std::setw(6) << inst.dst << std::setw(6) << inst.left; break;
            case OpCode::PRINT:
                std::cout << "PRINT" << std::setw(6) << inst.dst; break;
            case OpCode::CALL:
                std::cout << "CALL" << std::setw(6) << "-" << std::setw(6) << "-" 
                          << std::setw(6) << inst.dst << "ADDR: " << getAddress(inst); break;
            case OpCode::RETURN:
                std::cout << "RETURN" << std::setw(6) << inst.dst << std::setw(6) << "-" << std::setw(6) << "-"; break;
            case OpCode::PUSH_ARG:
                std::cout << "PUSH_ARG" << std::setw(6) << inst.dst << std::setw(6) << "-" << std::setw(6) << "-"; break;
            case OpCode::LOAD_PARAM:
                std::cout << "LOAD_PARAM" << std::setw(6) << inst.left << std::setw(6) << "-" << std::setw(6) << inst.dst; break;                
            default: std::cout << "UNKNOWN"; break;
        }
        std::cout << std::endl;
=======

        switch (op) {
            case OpCode::ADD: {
                // Check if either operand is a string
                if (isString(regs[inst.rs1]) || isString(regs[inst.rs2])) {
                    // String concatenation
                    std::string left = valueToString(regs[inst.rs1]);
                    std::string right = valueToString(regs[inst.rs2]);
                    regs[inst.rd] = left + right;
                } else {
                    // Numeric addition
                    regs[inst.rd] = asNumber(regs[inst.rs1]) + asNumber(regs[inst.rs2]);
                }
            }
            break;
            case OpCode::SUB: regs[inst.rd] = asNumber(regs[inst.rs1]) - asNumber(regs[inst.rs2]); break;
            case OpCode::MUL: regs[inst.rd] = asNumber(regs[inst.rs1]) * asNumber(regs[inst.rs2]); break;
            case OpCode::POW: regs[inst.rd] = std::pow(asNumber(regs[inst.rs1]), asNumber(regs[inst.rs2])); break;
            case OpCode::DIV: regs[inst.rd] = asNumber(regs[inst.rs1]) / asNumber(regs[inst.rs2]); break;
            case OpCode::AND: regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) & (long long)asNumber(regs[inst.rs2])); break;
            case OpCode::OR:  regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) | (long long)asNumber(regs[inst.rs2])); break;
            case OpCode::XOR: regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) ^ (long long)asNumber(regs[inst.rs2])); break;
            case OpCode::MOD: regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) % (long long)asNumber(regs[inst.rs2])); break;
            case OpCode::SLL: regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) << (long long)asNumber(regs[inst.rs2])); break;
            case OpCode::SRL: regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) >> (long long)asNumber(regs[inst.rs2])); break;

            case OpCode::ADDI: regs[inst.rd] = asNumber(regs[inst.rs1]) + (int16_t)inst.rs2; break;
            case OpCode::SUBI: regs[inst.rd] = asNumber(regs[inst.rs1]) - (int16_t)inst.rs2; break;
            case OpCode::MULI: regs[inst.rd] = asNumber(regs[inst.rs1]) * (int16_t)inst.rs2; break;
            case OpCode::DIVI: regs[inst.rd] = asNumber(regs[inst.rs1]) / (int16_t)inst.rs2; break;
            case OpCode::ANDI: regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) & (int16_t)inst.rs2); break;
            case OpCode::ORI:  regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) | (int16_t)inst.rs2); break;
            case OpCode::XORI: regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) ^ (int16_t)inst.rs2); break;
            case OpCode::MODI: regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) % (int16_t)inst.rs2); break;
            case OpCode::SLLI: regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) << (int16_t)inst.rs2); break;
            case OpCode::SRLI: regs[inst.rd] = (double)((long long)asNumber(regs[inst.rs1]) >> (int16_t)inst.rs2); break;

            case OpCode::SW: {
                int32_t addr = (int32_t)asNumber(regs[inst.rs1]) + (int16_t)inst.rs2;
                if (addr < 0 || addr >= (int32_t)memory.size()) 
                    throw std::runtime_error("SW: address out of range");
                        
                // Store the value directly (memory is vector<Value>)
                memory[addr] = regs[inst.rd];
                break;
            }
            
            case OpCode::LW: {
                int32_t addr = (int32_t)asNumber(regs[inst.rs1]) + (int16_t)inst.rs2;
                if (addr < 0 || addr >= (int32_t)memory.size()) 
                    throw std::runtime_error("LW: address out of range");
                regs[inst.rd] = memory[addr];
                break;
            }

            case OpCode::BEQ:
            if (asNumber(regs[inst.rd]) == asNumber(regs[inst.rs1])) {
                pc = getImmediate(inst);  // 16-bit absolute address
                jumped = true;
            }
                break;
            case OpCode::BNE:
                if (asNumber(regs[inst.rd]) != asNumber(regs[inst.rs1])) {
                    pc = getImmediate(inst);
                    jumped = true;
                }
                break;
            case OpCode::BLT:
                if (asNumber(regs[inst.rd]) < asNumber(regs[inst.rs1])) {
                    pc = getImmediate(inst);
                    jumped = true;
                }
                break;
            case OpCode::BGE:
                if (asNumber(regs[inst.rd]) >= asNumber(regs[inst.rs1])) {
                    pc = getImmediate(inst);
                    jumped = true;
                }
                break;
            
            case OpCode::JAL:
                if (inst.rd != Reg::ZERO) {
                    regs[inst.rd] = (double)(pc + 1);
                }
                pc = getImmediate(inst);  // 16-bit absolute address
                jumped = true;
                break;
            case OpCode::JALR:
                if (inst.rd != Reg::ZERO) {
                    regs[inst.rd] = (double)(pc + 1);
                }
                pc = (int16_t)asNumber(regs[inst.rs1]) + (int16_t)inst.rs2;
                jumped = true;
            break;

            case OpCode::LOAD_CONST: {
                int idx = inst.rs1;
                if (idx < 0 || idx >= (int)consts.size()) 
                    throw std::runtime_error("Constant pool index out of range");
                regs[inst.rd] = consts[idx];
                break;
            }
            case OpCode::LOAD_STR: {
                int idx = inst.rs1;
                if (idx < 0 || idx >= (int)strings.size()) 
                    throw std::runtime_error("String pool index out of range");
                regs[inst.rd] = strings[idx];
                break;
            }
            case OpCode::PRINT:
                std::visit([](auto&& val) { std::cout << val; }, regs[inst.rd]);
                break;
            case OpCode::PRINT_STR:
                std::cout << strings[inst.rd];
                break;

            case OpCode::LOGICAL_AND:
                regs[inst.rd] = (isTruthy(regs[inst.rs1]) && isTruthy(regs[inst.rs2])) ? 1.0 : 0.0;
                break;
            case OpCode::LOGICAL_OR:
                regs[inst.rd] = (isTruthy(regs[inst.rs1]) || isTruthy(regs[inst.rs2])) ? 1.0 : 0.0;
                break;
            case OpCode::LOGICAL_NOT:
                regs[inst.rd] = isFalsy(regs[inst.rs1]) ? 1.0 : 0.0;
                break;

            case OpCode::CMP_EQ: regs[inst.rd] = (regs[inst.rs1] == regs[inst.rs2]) ? 1.0 : 0.0; break;
            case OpCode::CMP_NE: regs[inst.rd] = (regs[inst.rs1] != regs[inst.rs2]) ? 1.0 : 0.0; break;
            case OpCode::CMP_LT: regs[inst.rd] = (asNumber(regs[inst.rs1]) <  asNumber(regs[inst.rs2])) ? 1.0 : 0.0; break;
            case OpCode::CMP_GT: regs[inst.rd] = (asNumber(regs[inst.rs1]) >  asNumber(regs[inst.rs2])) ? 1.0 : 0.0; break;
            case OpCode::CMP_LE: regs[inst.rd] = (asNumber(regs[inst.rs1]) <= asNumber(regs[inst.rs2])) ? 1.0 : 0.0; break;
            case OpCode::CMP_GE: regs[inst.rd] = (asNumber(regs[inst.rs1]) >= asNumber(regs[inst.rs2])) ? 1.0 : 0.0; break;

            case OpCode::HALT: 
                return asNumber(regs[A0]);

            default:
                throw std::runtime_error("Unknown opcode: " + std::to_string((int)op));
        }

        if (!jumped) ++pc;
    }

    return asNumber(regs[A0]);
}

void VirtualMachine::visualize() const {
    std::cout << "PC=" << pc << " SP=" << sp << " FP=" << fp << std::endl;
    for (int i = 0; i < 32; ++i) {
        if (i == ZERO) continue;
        if (std::holds_alternative<double>(regs[i]))
            std::cout << "x" << i << "=" << asNumber(regs[i]) << " ";
        else
            std::cout << "x" << i << "=\"" << asString(regs[i]) << "\" ";
        if ((i+1) % 8 == 0) std::cout << std::endl;
>>>>>>> 37c62253fa08934c2bae054db3a95e11c543af6e
    }
}