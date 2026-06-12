#include "ast.h"
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
    if(op == "~")  return OpCode::NOT;
    if(op == "%")  return OpCode::MODULO;
    if(op == "<<") return OpCode::SLL;
    if(op == ">>") return OpCode::SRL;
    if(op == ">")  return OpCode::CMP_GT;
    if(op == "<")  return OpCode::CMP_LT;
    if(op == ">=") return OpCode::CMP_GET;
    if(op == "<=") return OpCode::CMP_LET;
    if(op == "==") return OpCode::CMP_EQ;
    if(op == "!=") return OpCode::CMP_NEQ;
    if(op == "and") return OpCode::LOGICAL_AND;
    if(op == "or") return OpCode::LOGICAL_OR;
    if(op == "not") return OpCode::LOGICAL_NOT;
    return OpCode::UNDEFINED;
}
