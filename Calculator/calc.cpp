#include "calc.h"

void Calculate::visualize() {
    std::cout << "\n[SYMBOLIC Visualization]" << std::endl;
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
    std::cout << std::string(45, '-') << std::endl;
}

double Calculate::execute(const SymbolTable& symTable) {
    if(program.empty()) return 0.0;
        
    for(const auto& inst : program) {
        switch(inst.op) {
            case OpCode::LOAD_CONST : 
                registers[inst.dest] = inst.value;
            break;
            case OpCode::LOAD_VAR : 
                registers[inst.dest] = symTable.getValueByAddress(inst.left);
            break;
            case OpCode::ADD: 
                registers[inst.dest] = registers[inst.left] + registers[inst.right];
            break;
            case OpCode::SUB: 
                registers[inst.dest] = registers[inst.left] - registers[inst.right];
            break;
            case OpCode::DIV: {
                if(registers[inst.right] == 0) {
                    throw std::runtime_error("Division by zero.");
                }
                registers[inst.dest] = registers[inst.left] / registers[inst.right];
            }
            break;
            case OpCode::MUL: 
                registers[inst.dest] = registers[inst.left] * registers[inst.right];
            break;
            case OpCode::UNARY:
                registers[inst.dest] = -registers[inst.left];
            break;
            case OpCode::AND: {
                long long l_val = static_cast<long long>(registers[inst.left]);
                long long r_val = static_cast<long long>(registers[inst.right]);
                registers[inst.dest] = static_cast<double>(l_val & r_val);
            }
            break;
            case OpCode::OR: {
                long long l_val = static_cast<long long>(registers[inst.left]);
                long long r_val = static_cast<long long>(registers[inst.right]);
                registers[inst.dest] = static_cast<double>(l_val | r_val);
            }
            break;
            case OpCode::XOR: {
                long long l_val = static_cast<long long>(registers[inst.left]);
                long long r_val = static_cast<long long>(registers[inst.right]);
                registers[inst.dest] = static_cast<double>(l_val ^ r_val);
            }
            break;
            case OpCode::MODULO: {
                long long l_val = static_cast<long long>(registers[inst.left]);
                long long r_val = static_cast<long long>(registers[inst.right]);
                registers[inst.dest] = static_cast<double>(l_val % r_val);
            }
            break;
            case OpCode::LSHIFT: {
                long long l_val = static_cast<long long>(registers[inst.left]);
                long long r_val = static_cast<long long>(registers[inst.right]);
                registers[inst.dest] = static_cast<double>(l_val << r_val);
            }
            break;
            case OpCode::RSHIFT: {
                long long l_val = static_cast<long long>(registers[inst.left]);
                long long r_val = static_cast<long long>(registers[inst.right]);
                registers[inst.dest] = static_cast<double>(l_val >> r_val);
            }
            break;
        }
    }
    return registers[finalIdx] == -0.0 ? 0.0 : registers[finalIdx];
}

void Calculate::compile(const std::string& expr, SymbolTable& symTable) {

    std::cout<<"Compiling expression: "<<expr<<std::endl;
    program.clear();

    std::istringstream stream(expr);
    Lexer lexer(stream);
    Tokenizer tokenizer(lexer);
    Parser parser(tokenizer, symTable);

    auto root = parser.parse();
    if(!root) {
        std::cerr << "Compilation failed!" << std::endl;
        return;
    }

    root = root -> fold();
    CompileContext ctx;
    int tempCounter = 0;
    finalIdx = root -> transform(program, tempCounter, ctx);

    registers.assign(tempCounter, 0.0);

    if(debug_mode) {
        root -> print();
        std::cout<<"\nTransformed AST:"<<std::endl;
        visualize();
    }
}