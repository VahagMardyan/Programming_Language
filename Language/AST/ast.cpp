#include "ast.h"

void NumberNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "Number: " << value << std::endl;
}

OpCode BinaryOpNode::getOpCode() const {
    if(op == "+")  return OpCode::ADD;
    if(op == "-")  return OpCode::SUB;
    if(op == "/")  return OpCode::DIV;
    if(op == "//") return OpCode::FLOOR_DIV;
    if(op == "%/") return OpCode::FRAC_DIV;
    if(op == "*")  return OpCode::MUL;
    if(op == "**") return OpCode::POW;
    if(op == "&")  return OpCode::AND;
    if(op == "|")  return OpCode::OR;
    if(op == "^")  return OpCode::XOR;
    if(op == "%")  return OpCode::MODULO;
    if(op == "<<") return OpCode::SLL;
    if(op == ">>") return OpCode::SRL;
    if(op == ">")  return OpCode::CMP_GT;
    if(op == "<")  return OpCode::SLT;
    if(op == ">=") return OpCode::CMP_GET;
    if(op == "<=") return OpCode::CMP_LET;
    if(op == "==") return OpCode::CMP_EQ;
    if(op == "!=") return OpCode::CMP_NEQ;
    if(op == "and") return OpCode::LOGICAL_AND;
    if(op == "or") return OpCode::LOGICAL_OR;
    if(op == "not") return OpCode::LOGICAL_NOT;
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
    
    for(size_t i = 0; i < expressions.size(); ++i) {
        bool last = (i == expressions.size() - 1);
        expressions[i]->print(prefix + (isLast ? "    " : "│   "), last);
        if (!last) {
            std::cout << " ";
        }
    }
}

void ForStatementNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "For" << std::endl;
    std::string p = prefix + (isLast ? "    " : "│   ");
    init->print(p, false);
    condition->print(p, false);
    update->print(p, false);
    body->print(p, true);
}

void FunctionDefNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ")
              << "Function: " << name << "(";
    for(size_t i = 0; i < params.size(); ++i) {
        std::cout << params[i];
        if(i < params.size()-1) std::cout << ", ";
    }
    std::cout << ")" << std::endl;
    body->print(prefix + (isLast ? "    " : "│   "), true);
}

void FunctionCallNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ")
              << "Call: " << name << std::endl;
    for(size_t i = 0; i < args.size(); ++i)
        args[i]->print(prefix + (isLast ? "    " : "│   "), i == args.size()-1);
}

void ReturnNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "Return" << std::endl;
    if(expression)
        expression->print(prefix + (isLast ? "    " : "│   "), true);
}