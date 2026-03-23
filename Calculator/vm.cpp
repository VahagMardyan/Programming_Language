#include "vm.h"

void VirtualMachine::load(const std::string& expr, SymbolTable& symtable) {
    std::cout << "Loading expression: " << expr << std::endl;
    program.clear();
    std::istringstream stream(expr);
    Lexer lexer(stream);
    Tokenizer tokenizer(lexer);
    Parser parser(tokenizer, symtable);
    
    auto root = parser.parse();
    if(!root) {
        throw std::runtime_error("Parsing failed!");
    }
    Compiler compiler;
    program = compiler.compile(root);

    int maxReg = 0;
    for(const auto& inst : program) {
        maxReg = std::max({maxReg, inst.left, inst.right, inst.dest});
    }

    registers.assign(maxReg + 1, 0.0);

    finalIdx = program.empty() ? 0 : program.back().dest;

    if(debug_mode) {
        root -> print();
        visualize();
    }
}

double VirtualMachine::run(const SymbolTable& symTable) {
    if (program.empty()) return 0.0;

    for (const auto& inst : program) {
        switch (inst.op) {
            case OpCode::LOAD_CONST:
                registers[inst.dest] = inst.value;
                break;
            case OpCode::LOAD_VAR:
                registers[inst.dest] = symTable.getValueByAddress(inst.left);
                break;
            case OpCode::ADD:
                registers[inst.dest] = registers[inst.left] + registers[inst.right];
                break;
            case OpCode::SUB:
                registers[inst.dest] = registers[inst.left] - registers[inst.right];
                break;
            case OpCode::MUL:
                registers[inst.dest] = registers[inst.left] * registers[inst.right];
                break;
            case OpCode::DIV:
                if (registers[inst.right] == 0) throw std::runtime_error("Division by zero");
                registers[inst.dest] = registers[inst.left] / registers[inst.right];
                break;
            case OpCode::UNARY:
                registers[inst.dest] = -registers[inst.left];
                break;
            case OpCode::MODULO:
                registers[inst.dest] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) % 
                    static_cast<long long>(registers[inst.right]));
                break;

            case OpCode::AND: 
                registers[inst.dest] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) & 
                    static_cast<long long>(registers[inst.right]));   
            break;

            case OpCode::OR: 
                registers[inst.dest] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) | 
                    static_cast<long long>(registers[inst.right]));   
            break;
            
            case OpCode::XOR: 
                registers[inst.dest] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) ^ 
                    static_cast<long long>(registers[inst.right]));   
            break;

            case OpCode::LSHIFT: 
                registers[inst.dest] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) << 
                    static_cast<long long>(registers[inst.right]));   
            break;

            case OpCode::RSHIFT: 
                registers[inst.dest] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) >>
                    static_cast<long long>(registers[inst.right]));   
            break;

            default: break;
        }
    }
    return registers[finalIdx] == -0.0 ? 0.0 : registers[finalIdx];
}

void VirtualMachine::visualize() const {
    std::cout << "\n[VM Bytecode Visualization]" << std::endl;
    std::cout << std::left << std::setw(6)  << "Addr" 
              << std::setw(12) << "OpCode" 
              << std::setw(6)  << "L" 
              << std::setw(6)  << "R" 
              << std::setw(6)  << "Dst" 
              << "Value" << std::endl;
    std::cout << std::string(45, '-') << std::endl;

    for (size_t i = 0; i < program.size(); ++i) {
        const auto& inst = program[i];
        std::cout << "[" << std::setw(3) << i << "]  ";
        
        std::cout << std::left << std::setw(12);

        switch (inst.op) {
            case OpCode::LOAD_CONST:
                std::cout << "LOAD_CONST" << std::setw(6) << "-" << std::setw(6) << "-" 
                          << std::setw(6) << inst.dest << inst.value;
                break;
            case OpCode::LOAD_VAR:
                std::cout << "LOAD_VAR" << std::setw(6) << inst.left << std::setw(6) << "-"
                          << std::setw(6) << inst.dest << "*(" << inst.dest << ")";
                break;
            case OpCode::ADD:
                std::cout << "ADD" << std::setw(6) << inst.left << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dest << "*(" << inst.left << ") + (" << "*" << inst.right << ")";
                break;
            case OpCode::SUB:
                std::cout << "SUB" << std::setw(6) << inst.left << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dest << "*(" << inst.left << ") - (" << "*" << inst.right << ")";
                break;
            case OpCode::MUL:
                std::cout << "MUL" << std::setw(6) << inst.left << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dest << "*(" << inst.left << ") * (" << "*" << inst.right << ")";
                break;
            case OpCode::DIV:
                std::cout << "DIV" << std::setw(6) << inst.left << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dest << "*(" << inst.left << ") / (" << "*" << inst.right << ")";
                break;
            case OpCode::UNARY:
                std::cout << "NEG" << std::setw(6) << inst.left << std::setw(6) << "-" 
                          << std::setw(6) << inst.dest << "-" << inst.dest;
                break;
            case OpCode::AND:
                std::cout << std::setw(12) << "AND" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dest << "*(" << inst.left << ") & (" << "*" << inst.right << ")";
                break;
            case OpCode::OR:
                std::cout << std::setw(12) << "OR" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dest << "*(" << inst.left << ") | (" << "*" << inst.right << ")";
                break;
            case OpCode::XOR:
                std::cout << std::setw(12) << "XOR" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dest << "*(" << inst.left << ") ^ (" << "*" << inst.right << ")";
                break;
            case OpCode::MODULO:
                std::cout << std::setw(12) << "MODULO" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dest << "*(" << inst.left << ") % (" << "*" << inst.right << ")";
                break;
            case OpCode::LSHIFT:
                std::cout << std::setw(12) << "LSHIFT" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dest << "*(" << inst.left << ") << (" << "*" << inst.right << ")";;
                break;
            case OpCode::RSHIFT:
                std::cout << std::setw(12) << "RSHIFT" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dest << "*(" << inst.left << ") >> (" << "*" << inst.right << ")";
                break;
        }
        std::cout << std::endl;
    }
}