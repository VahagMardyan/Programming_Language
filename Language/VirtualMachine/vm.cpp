#include "vm.h"

void VirtualMachine::load(const std::string& expr, SymbolTable& symtable) {
    // std::cout << "Loading expression: " << expr << std::endl; // For Debugging
    std::istringstream stream(expr);
    Lexer lexer(stream);
    Tokenizer tokenizer(lexer);
    Parser parser(tokenizer, symtable);

    auto root = std::static_pointer_cast<ASTNode>(parser.parseProgram());
    if(!root) throw std::runtime_error("Parsing failed!");

    Compiler compiler;
    ByteCode bc = compiler.compile(root);
    current_program  = bc.instructions;
    current_consants = bc.constants;
    current_strings = bc.strings;

    int maxReg = 0;
    for(const auto& inst : current_program)
        maxReg = std::max({maxReg, (int)inst.left, (int)inst.right, (int)inst.dst});
    registers.assign(std::max(256, maxReg + 1), 0.0);

    if(debug_mode) {
        root->print();
        visualize(current_program);
    }
}

double VirtualMachine::run(SymbolTable& st) {
    size_t pc = 0;
    int lastDest = 0;
    while(pc < current_program.size()) {
        const auto& inst = current_program[pc];
        OpCode op = static_cast<OpCode>(inst.op);
        lastDest = inst.dst;
        bool jumped = false;
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
              << std::setw(12) << "OpCode"
              << std::setw(6)  << "L"
              << std::setw(6)  << "R"
              << std::setw(6)  << "Dst"
              << "Value" << std::endl;
    std::cout << std::string(45, '-') << std::endl;

    for(size_t i = 0; i < program.size(); ++i) {
        const auto& inst = program[i];
        OpCode op = static_cast<OpCode>(inst.op);
        std::cout << "[" << std::setw(3) << i << "]  ";
        std::cout << std::left << std::setw(12);

        switch(op) {
            case OpCode::LOAD_CONST:
                std::cout << "LOAD_CONST" << std::setw(6) << "-" << std::setw(6) << "-"
                          << std::setw(6) << inst.dst << current_consants[inst.left]; break;
            case OpCode::LOAD_VAR:
                std::cout << "LOAD_VAR" << std::setw(6) << inst.left << std::setw(6) << "-"
                          << std::setw(6) << inst.dst; break;
            case OpCode::LOAD_STR:
                std::cout << "LOAD_STR" << std::setw(6) << inst.left << std::setw(6) << "-"
                          << std::setw(6) << inst.dst << "\"" << current_strings[inst.left] << "\""; 
            break;
            case OpCode::STORE_VAR:
                std::cout << "STORE_VAR" << std::setw(6) << inst.left << std::setw(6) << inst.right
                          << std::setw(6) << "-" << "mem[" << inst.left << "] = r" << inst.right; break;
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
            default: std::cout << "UNKNOWN"; break;
        }
        std::cout << std::endl;
    }
}