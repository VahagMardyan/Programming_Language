#include "ast.h"

// NumberNode
void NumberNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "Number: " << value << std::endl;
}

// VariableNode
void VariableNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "Var (Addr: " << address << ")" << std::endl;
}

// BinaryOpNode
OpCode BinaryOpNode::getOpCode() const {
    if(op == "+") return OpCode::ADD;
    if(op == "-") return OpCode::SUB;
    if(op == "/") return OpCode::DIV;
    if(op == "*") return OpCode::MUL;
    if(op == "&") return OpCode::AND;
    if(op == "|") return OpCode::OR;
    if(op == "^") return OpCode::XOR;
    if(op == "%") return OpCode::MODULO;
    if(op == "<<") return OpCode::LSHIFT;
    if(op == ">>") return OpCode::RSHIFT;
    return OpCode::UNDEFINED;
}

void BinaryOpNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "BinaryOp: " << op << std::endl;

    std::string newPrefix = prefix + (isLast ? "    " : "│   ");
    left->print(newPrefix, false);
    right->print(newPrefix, true);
}

// UnaryOpNode
void UnaryOpNode::print(std::string prefix, bool isLast) const {
    std::string sign = (op == "-" || op == "_") ? "-" : "+";
    std::cout << prefix << (isLast ? "└── " : "├── ") << "UnaryOp: " << sign << std::endl;

    std::string newPrefix = prefix + (isLast ? "    " : "│   ");
    child -> print(newPrefix, true);
}
