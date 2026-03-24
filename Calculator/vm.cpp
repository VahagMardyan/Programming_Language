#include "vm.h"

void VirtualMachine::load(const std::string& expr, SymbolTable& symtable) {
    std::cout << "Loading expression: " << expr << std::endl;
    std::istringstream stream(expr);
    Lexer lexer(stream);
    Tokenizer tokenizer(lexer);
    Parser parser(tokenizer, symtable);

    auto root = parser.parse();
    if(!root) {
        throw std::runtime_error("Parsing failed!");
    }
    
    Compiler compiler;
    auto optimizedRoot = compiler.optimize(root);
    ByteCode bc = compiler.compile(optimizedRoot);

    this -> current_program = bc.instructions;
    this -> current_consants = bc.constants;

    int maxReg = 0;
    for(const auto& inst : current_program) {
        maxReg = std::max({
            maxReg, (int)inst.left, (int)inst.right, (int)inst.dst
        });
    }
    registers.assign(std::max(256, maxReg + 1), 0.0);

    if(debug_mode) {
        optimizedRoot -> print();
        visualize(current_program);
    }
}

double VirtualMachine::run(SymbolTable& st) {
    int lastDest = 0;
    for(const auto& inst : current_program) {
        OpCode op = static_cast<OpCode>(inst.op);
        lastDest = inst.dst;
        switch(op) {
            case OpCode::LOAD_CONST:
                registers[inst.dst] = current_consants[inst.left];
            break;
            case OpCode::LOAD_VAR:
                registers[inst.dst] = st.getValueByAddress(inst.left);
            break;
            case OpCode::ADD:
                registers[inst.dst] = registers[inst.left] + registers[inst.right];
            break;
            case OpCode::SUB:
                registers[inst.dst] = registers[inst.left] - registers[inst.right];
            break;
            case OpCode::MUL:
                registers[inst.dst] = registers[inst.left] * registers[inst.right];
            break;
            case OpCode::DIV:
                if (registers[inst.right] == 0) throw std::runtime_error("Division by zero");
                registers[inst.dst] = registers[inst.left] / registers[inst.right];
            break;
            case OpCode::UNARY:
                registers[inst.dst] = -registers[inst.left];
            break;
            case OpCode::MODULO:
                registers[inst.dst] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) % 
                    static_cast<long long>(registers[inst.right]));
                break;

            case OpCode::AND: 
                registers[inst.dst] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) & 
                    static_cast<long long>(registers[inst.right]));   
            break;

            case OpCode::OR: 
                registers[inst.dst] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) | 
                    static_cast<long long>(registers[inst.right]));   
            break;
            
            case OpCode::XOR: 
                registers[inst.dst] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) ^ 
                    static_cast<long long>(registers[inst.right]));   
            break;

            case OpCode::LSHIFT: 
                registers[inst.dst] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) << 
                    static_cast<long long>(registers[inst.right]));   
            break;

            case OpCode::RSHIFT: 
                registers[inst.dst] = static_cast<double>(
                    static_cast<long long>(registers[inst.left]) >>
                    static_cast<long long>(registers[inst.right]));   
            break;

            default: break;
        }
    }
    return registers[lastDest] == -0.0 ? 0.0 : registers[lastDest];
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

    for (size_t i = 0; i < program.size(); ++i) {
        const auto& inst = program[i];
        OpCode op = static_cast<OpCode>(inst.op);
        std::cout << "[" << std::setw(3) << i << "]  ";
        
        std::cout << std::left << std::setw(12);

        switch (op) {
            case OpCode::LOAD_CONST:
                std::cout << "LOAD_CONST" << std::setw(6) << "-" << std::setw(6) << "-" 
                          << std::setw(6) << inst.dst << current_consants[inst.left];
                break;
            case OpCode::LOAD_VAR:
                std::cout << "LOAD_VAR" << std::setw(6) << inst.left << std::setw(6) << "-"
                          << std::setw(6) << inst.dst << "*(" << inst.dst << ")";
                break;
            case OpCode::ADD:
                std::cout << "ADD" << std::setw(6) << inst.left << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dst << "*(" << inst.left << ") + (" << "*" << inst.right << ")";
                break;
            case OpCode::SUB:
                std::cout << "SUB" << std::setw(6) << inst.left << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dst << "*(" << inst.left << ") - (" << "*" << inst.right << ")";
                break;
            case OpCode::MUL:
                std::cout << "MUL" << std::setw(6) << inst.left << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dst << "*(" << inst.left << ") * (" << "*" << inst.right << ")";
                break;
            case OpCode::DIV:
                std::cout << "DIV" << std::setw(6) << inst.left << std::setw(6) << inst.right 
                          << std::setw(6) << inst.dst << "*(" << inst.left << ") / (" << "*" << inst.right << ")";
                break;
            case OpCode::UNARY:
                std::cout << "NEG" << std::setw(6) << inst.left << std::setw(6) << "-" 
                          << std::setw(6) << inst.dst << "-" << inst.dst;
                break;
            case OpCode::AND:
                std::cout << std::setw(12) << "AND" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dst << "*(" << inst.left << ") & (" << "*" << inst.right << ")";
                break;
            case OpCode::OR:
                std::cout << std::setw(12) << "OR" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dst << "*(" << inst.left << ") | (" << "*" << inst.right << ")";
                break;
            case OpCode::XOR:
                std::cout << std::setw(12) << "XOR" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dst << "*(" << inst.left << ") ^ (" << "*" << inst.right << ")";
                break;
            case OpCode::MODULO:
                std::cout << std::setw(12) << "MODULO" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dst << "*(" << inst.left << ") % (" << "*" << inst.right << ")";
                break;
            case OpCode::LSHIFT:
                std::cout << std::setw(12) << "LSHIFT" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dst << "*(" << inst.left << ") << (" << "*" << inst.right << ")";;
                break;
            case OpCode::RSHIFT:
                std::cout << std::setw(12) << "RSHIFT" << std::setw(6) << inst.left << std::setw(6)
                        << inst.right << std::setw(6) << inst.dst << "*(" << inst.left << ") >> (" << "*" << inst.right << ")";
                break;
        }
        std::cout << std::endl;
    }
}