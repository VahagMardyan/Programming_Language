#include "vm.h"
#include <cmath>

namespace {
int32_t toInt32(const Value& v) {
    return static_cast<int32_t>(asNumber(v));
}

double fromInt32(int32_t v) {
    return static_cast<double>(v);
}
}

void VirtualMachine::load(const std::string& expr, SymbolTable& symtable) {
    std::istringstream stream(expr);
    Lexer lexer(stream);
    Tokenizer tokenizer(lexer);
    Parser parser(tokenizer, symtable);

    auto root = std::static_pointer_cast<ASTNode>(parser.parseProgram());
    if(!root) throw std::runtime_error("Parsing failed!");
    Compiler compiler(symtable);
    ByteCode bc = compiler.compile(root);
    loadByteCode(bc);

    if(debug_mode) {
        root->print();
        visualize(current_program);
    }
}

void VirtualMachine::loadFromFile(const std::string& byteCodePath) {
    ByteCode bc = readByteCodeFromFile(byteCodePath);
    loadByteCode(bc);
    if(debug_mode) {
        visualize(current_program);
    }
}

void VirtualMachine::loadByteCode(const ByteCode& bc) {
    current_program  = bc.instructions;
    current_consants = bc.constants;
    current_strings = bc.strings;

    int maxReg = 0;
    for(const auto& inst : current_program)
        maxReg = std::max({maxReg, (int)inst.left, (int)inst.right, (int)inst.dst});
    const int kRv32RegCount = 32;
    const int kSpReg = 2; // x2 (sp)
    const int kFpReg = 8; // x8 (s0/fp)
    registers.assign(std::max(kRv32RegCount, maxReg + 1), 0.0);
    registers[kSpReg] = 10000.0;
    registers[kFpReg] = 10000.0;
    memory.resize(65536);
}

double VirtualMachine::run() {
    size_t pc = 0;
    int lastDest = 0;
    while(pc < current_program.size()) {
        const auto& inst = current_program[pc];
        OpCode op = static_cast<OpCode>(inst.op);
        lastDest = inst.dst;
        bool jumped = false;
        switch(op) {
            case OpCode::LOAD_CONST: registers[inst.dst] = current_consants[inst.left]; break;
            case OpCode::LOAD_VAR:   registers[inst.dst] = memory[inst.left]; break;
            case OpCode::LOAD_STR: registers[inst.dst] = current_strings[inst.left]; break;
            case OpCode::STORE_VAR:  memory[inst.left] = registers[inst.right]; break;
            
            case OpCode::ADD:  {
                if(isString(registers[inst.left]) || isString(registers[inst.right])) {
                    registers[inst.dst] = valueToString(registers[inst.left]) + valueToString(registers[inst.right]);
            } else {
                    registers[inst.dst] = asNumber(registers[inst.left]) + asNumber(registers[inst.right]);
            }
        }
        break;
        case OpCode::SUB:    registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) - toInt32(registers[inst.right])); break;
        case OpCode::MUL:    registers[inst.dst] = asNumber(registers[inst.left]) * asNumber(registers[inst.right]); break;
        case OpCode::POW:    registers[inst.dst] = std::pow(asNumber(registers[inst.left]), asNumber(registers[inst.right])); break;
        case OpCode::DIV:
            if(asNumber(registers[inst.right]) == 0) throw std::runtime_error("Division by zero");
            registers[inst.dst] = asNumber(registers[inst.left]) / asNumber(registers[inst.right]); break;
        case OpCode::UNARY:  registers[inst.dst] = -asNumber(registers[inst.left]); break;
        case OpCode::MODULO: registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) % toInt32(registers[inst.right])); break;
        case OpCode::AND:    registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) & toInt32(registers[inst.right])); break;
        case OpCode::OR:     registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) | toInt32(registers[inst.right])); break;
        case OpCode::XOR:    registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) ^ toInt32(registers[inst.right])); break;
        case OpCode::SLL:    registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) << (toInt32(registers[inst.right]) & 0x1F)); break;
        case OpCode::SRL:    registers[inst.dst] = fromInt32((uint32_t)toInt32(registers[inst.left]) >> (toInt32(registers[inst.right]) & 0x1F)); break;
        case OpCode::SRA:    registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) >> (toInt32(registers[inst.right]) & 0x1F)); break;
        case OpCode::SLT:    registers[inst.dst] = toInt32(registers[inst.left]) < toInt32(registers[inst.right]) ? 1.0 : 0.0; break;
        case OpCode::SLTU:   registers[inst.dst] = (uint32_t)toInt32(registers[inst.left]) < (uint32_t)toInt32(registers[inst.right]) ? 1.0 : 0.0; break;
        case OpCode::CMP_LT: registers[inst.dst] = toInt32(registers[inst.left]) < toInt32(registers[inst.right]) ? 1.0 : 0.0; break;
        case OpCode::CMP_GT:  registers[inst.dst] = asNumber(registers[inst.left]) >  asNumber(registers[inst.right]) ? 1.0 : 0.0; break;
        case OpCode::CMP_GET: registers[inst.dst] = asNumber(registers[inst.left]) >= asNumber(registers[inst.right]) ? 1.0 : 0.0; break;
        case OpCode::CMP_LET: registers[inst.dst] = asNumber(registers[inst.left]) <= asNumber(registers[inst.right]) ? 1.0 : 0.0; break;
        case OpCode::CMP_EQ:  registers[inst.dst] = registers[inst.left] == registers[inst.right] ? 1.0 : 0.0; break;
        case OpCode::CMP_NEQ: registers[inst.dst] = registers[inst.left] != registers[inst.right] ? 1.0 : 0.0; break;
        case OpCode::JZ: {
                if(isFalsy(registers[inst.dst])) {
                    pc = getAddress(inst);
                    jumped = true;
            }
        }
        break;
        case OpCode::JMP: pc = getAddress(inst); jumped = true;
        break;
        case OpCode::PRINT: {
            const Value& val = registers[inst.dst];
            if (isString(val)) {
                std::cout << asString(val);
            }
            else if (isNumber(val)) {
                double num = asNumber(val);
                if (num == (long long)num) {
                    std::cout << (long long)num;
                } else {
                    std::cout << num;
                }
            }
            else {
                std::cout << valueToString(val);
            }
            break;
        }
        case OpCode::PRINT_STR: std::cout<<current_strings[inst.dst]; break;
        
        case OpCode::LOGICAL_AND: registers[inst.dst] = (isTruthy(registers[inst.left]) && isTruthy(registers[inst.right])) ? 1.0 : 0.0; break;
        case OpCode::LOGICAL_OR: registers[inst.dst] = (isTruthy(registers[inst.left]) || isTruthy(registers[inst.right])) ? 1.0 : 0.0; break;
        case OpCode::LOGICAL_NOT: registers[inst.dst] = isFalsy(registers[inst.left]) ? 0.0 : 1.0; break;

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

        case OpCode::LOAD: {
            int32_t base   = (int32_t)asNumber(registers[inst.left]); // FP
            int32_t offset = (int32_t)inst.right;
            int32_t addr   = base + offset;

            if (addr < 0 || addr >= (int32_t)memory.size()) {
                std::cerr << "LOAD out of range: addr=" << addr << std::endl;
                throw std::runtime_error("LOAD out of range");
            }
            registers[inst.dst] = memory[addr];
            break;
        }

        case OpCode::STORE: {
            int32_t base   = (int32_t)asNumber(registers[inst.left]); // FP
            int32_t offset = (int32_t)inst.right;
            int32_t addr   = base + offset;

            if (addr < 0 || addr >= (int32_t)memory.size()) {
                std::cerr << "STORE out of range: addr=" << addr << std::endl;
                throw std::runtime_error("STORE out of range");
            }
            memory[addr] = registers[inst.dst];
            break;
        }

        case OpCode::ADDI: {
            int8_t signedImm = static_cast<int8_t>(inst.right);
            int32_t imm = signedImm;
            registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) + imm);
            break;
        }
        case OpCode::ANDI: {
            int8_t signedImm = static_cast<int8_t>(inst.right);
            registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) & signedImm);
            break;
        }
        case OpCode::ORI: {
            int8_t signedImm = static_cast<int8_t>(inst.right);
            registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) | signedImm);
            break;
        }
        case OpCode::XORI: {
            int8_t signedImm = static_cast<int8_t>(inst.right);
            registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) ^ signedImm);
            break;
        }
        case OpCode::SLLI: {
            registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) << (inst.right & 0x1F));
            break;
        }
        case OpCode::SRLI: {
            registers[inst.dst] = fromInt32((uint32_t)toInt32(registers[inst.left]) >> (inst.right & 0x1F));
            break;
        }
        case OpCode::SRAI: {
            registers[inst.dst] = fromInt32(toInt32(registers[inst.left]) >> (inst.right & 0x1F));
            break;
        }
        case OpCode::LW:
            registers[inst.dst] = memory[inst.left];
            break;
        case OpCode::SW:
            memory[inst.left] = registers[inst.dst];
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
              << std::setw(6)  << "R" << std::setw(6)  << "Dst" << " Value" << std::endl; 
    std::cout << std::string(50, '-') << std::endl;

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
                          << std::setw(6) << "-" << " mem[" << inst.left << "] = r" << inst.right;
                break;

            case OpCode::ADD:     std::cout << "ADD";     break;
            case OpCode::SUB:     std::cout << "SUB";     break;
            case OpCode::MUL:     std::cout << "MUL";     break;
            case OpCode::DIV:     std::cout << "DIV";     break;
            case OpCode::POW:     std::cout << "POW";     break;
            case OpCode::MODULO:  std::cout << "MODULO";  break;
            case OpCode::UNARY:   std::cout << "NEG";     break;

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

            default:
                std::cout << "UNKNOWN (op=" << (int)op << ")";
                break;
        }
        std::cout << std::endl;
    }
}