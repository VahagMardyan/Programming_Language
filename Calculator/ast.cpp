#include "ast.h"

// NumberNode
int NumberNode::transform(std::vector<Instruction> &program, int &tempCounter, CompileContext &ctx) const {
    if (ctx.consts.find(value) != ctx.consts.end())
    {
        return ctx.consts[value];
    }

    int dest = tempCounter++;
    program.push_back({OpCode::LOAD_CONST, 0, 0, dest, value});

    ctx.consts[value] = dest;

    return dest;
}

void NumberNode::print(std::string prefix, bool isLast) const {
    std::cout << prefix << (isLast ? "└── " : "├── ") << "Number: " << value << std::endl;
}

double NumberNode::getValue() const {
    return value;
}

// VariableNode
int VariableNode::transform(std::vector<Instruction> &program, int &tempCounter, CompileContext &ctx) const {
    if (ctx.vars.find(address) != ctx.vars.end())
    {
        return ctx.vars[address];
    }
    int dest = tempCounter++;
    program.push_back({OpCode::LOAD_VAR, (int)address, 0, dest, 0.0});
    ctx.vars[address] = dest;
    return dest;
}

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

int BinaryOpNode::transform(std::vector<Instruction> &program, int &tempCounter, CompileContext &ctx) const {
    int l_idx = left->transform(program, tempCounter, ctx);
    int r_idx = right->transform(program, tempCounter, ctx);

    int dest = tempCounter++;
    program.push_back({getOpCode(), l_idx, r_idx, dest, 0.0});
    return dest;
}

std::shared_ptr<ASTNode> BinaryOpNode::fold() {
    left = left->fold();
    right = right->fold();

    auto leftNum = std::dynamic_pointer_cast<NumberNode>(left);
    auto rightNum = std::dynamic_pointer_cast<NumberNode>(right);
    if (leftNum && rightNum) {
        double lVal = leftNum->getValue();
        double rVal = rightNum->getValue();
        double res = 0;
        if (op == "+")
            res = lVal + rVal;
        else if (op == "-")
            res = lVal - rVal;
        else if (op == "*")
            res = lVal * rVal;
        else if (op == "/") {
            if (rVal != 0)
                res = lVal / rVal;
            else
                res = 0;
        }
        else if (op == "&") {
            long long l = static_cast<long long>(lVal);
            long long r = static_cast<long long>(rVal);
            res = static_cast<double>(l & r);
        }
        else if (op == "|") {
            long long l = static_cast<long long>(lVal);
            long long r = static_cast<long long>(rVal);
            res = static_cast<double>(l | r);
        }
        else if (op == "^") {
            long long l = static_cast<long long>(lVal);
            long long r = static_cast<long long>(rVal);
            res = static_cast<double>(l ^ r);
        }
        else if (op == "%") {
            long long l = static_cast<long long>(lVal);
            long long r = static_cast<long long>(rVal);
            res = static_cast<double>(l % r);
        }
        else if (op == "<<") {
            long long l = static_cast<long long>(lVal);
            long long r = static_cast<long long>(rVal);
            res = static_cast<double>(l << r);
        }
        else if (op == ">>") {
            long long l = static_cast<long long>(lVal);
            long long r = static_cast<long long>(rVal);
            res = static_cast<double>(l >> r);
        }
        return std::make_shared<NumberNode>(res);
    }
    return shared_from_this();
}

// UnaryOpNode
void UnaryOpNode::print(std::string prefix, bool isLast) const {
    std::string sign = (op == "-" || op == "_") ? "-" : "+";
    std::cout << prefix << (isLast ? "└── " : "├── ") << "UnaryOp: " << sign << std::endl;

    std::string newPrefix = prefix + (isLast ? "    " : "│   ");
    child -> print(newPrefix, true);
}

int UnaryOpNode::transform(std::vector<Instruction>& program, int& tempCounter, CompileContext& ctx) const {
    int c_idx = child -> transform(program, tempCounter, ctx);
    if(op == "-" || op == "_") {
        int dest = tempCounter++;
        program.push_back({
            OpCode::UNARY, c_idx, 0, dest, 0.0
        });
        return dest;
    }
    return c_idx;
}

std::shared_ptr<ASTNode> UnaryOpNode::fold() {
    child = child -> fold();
    auto num = std::dynamic_pointer_cast<NumberNode>(child);
    if(num) {
        double val = num -> getValue();
        if(op == "_") {
            return std::make_shared<NumberNode>(-val);
        }
        if(op == "#") {
            return std::make_shared<NumberNode>(val);
        }
    }
    return shared_from_this();
}