#include "ast.h"

void NumberNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "Number: " << value << std::endl;
}

void VariableNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "Var (Addr: " << address << ")" << std::endl;
}

OpCode BinaryOpNode::getOpCode() const {
    if(op == "+")  return OpCode::ADD;
    if(op == "-")  return OpCode::SUB;
    if(op == "/")  return OpCode::DIV;
    if(op == "*")  return OpCode::MUL;
    if(op == "&")  return OpCode::AND;
    if(op == "|")  return OpCode::OR;
    if(op == "^")  return OpCode::XOR;
    if(op == "%")  return OpCode::MODULO;
    if(op == "<<") return OpCode::LSHIFT;
    if(op == ">>") return OpCode::RSHIFT;
    if(op == ">")  return OpCode::CMP_GT;
    if(op == "<")  return OpCode::CMP_LT;
    if(op == ">=") return OpCode::CMP_GET;
    if(op == "<=") return OpCode::CMP_LET;
    if(op == "==") return OpCode::CMP_EQ;
    if(op == "!=") return OpCode::CMP_NEQ;
    return OpCode::UNDEFINED;
}

void BinaryOpNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "BinaryOp: " << op << std::endl;
    std::string newPrefix = prefix + (isLast ? "    " : "│   ");
    left->print(newPrefix, false);
    right->print(newPrefix, true);
}

void UnaryOpNode::print(std::string prefix, bool isLast) const {
    std::string sign = (op == "-" || op == "_") ? "-" : "+";
    std::cout << prefix << (isLast ? "└── " : "├── ") << "UnaryOp: " << sign << std::endl;
    std::string newPrefix = prefix + (isLast ? "    " : "│   ");
    child->print(newPrefix, true);
}

void AssignmentNode::print(std::string indent, bool isLast) const {
    std::cout << indent << (isLast ? "└── " : "├── ") << "Assignment (=)" << std::endl;
    expression->print(indent + (isLast ? "    " : "│   "), true);
}

void IfStatementNode::print(std::string indent, bool isLast) const {
    std::cout << indent << (isLast ? "└── " : "├── ") << "If" << std::endl;
    condition->print(indent + (isLast ? "    " : "│   "), false);
    thenBranch->print(indent + (isLast ? "    " : "│   "), elseBranch == nullptr);
    if(elseBranch) elseBranch->print(indent + (isLast ? "    " : "│   "), true);
}

void WhileStatementNode::print(std::string indent, bool isLast) const {
    std::cout << indent << (isLast ? "└── " : "├── ") << "While" << std::endl;
    condition->print(indent + (isLast ? "    " : "│   "), false);
    body->print(indent + (isLast ? "    " : "│   "), true);
}

void BlockCode::print(std::string indent, bool isLast) const {
    std::cout << indent << (isLast ? "└── " : "├── ") << "Block" << std::endl;
    for(size_t i = 0; i < statements.size(); ++i)
        statements[i]->print(indent + (isLast ? "    " : "│   "), i == statements.size()-1);
}

void PrintNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "Print" << std::endl;
    for(size_t i = 0; i < expressions.size(); ++i)
        expressions[i]->print(prefix + (isLast ? "    " : "│   "), i == expressions.size()-1);
}