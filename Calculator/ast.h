#pragma once

#include <iostream>
#include <map>
#include "symbol_table.h"

enum class OpCode {
    ADD, SUB, MUL, DIV, 
    AND, OR, XOR, MODULO, 
    LSHIFT, RSHIFT,
    UNARY,
    LOAD_CONST,
    LOAD_VAR,
    UNDEFINED,
};

struct Instruction {
    OpCode op;
    int left;
    int right;
    int dest; // index
    double value;
};

struct CompileContext {
    std::map<size_t, int> vars;
    std::map<double, int> consts;
};

class ASTNode : public std::enable_shared_from_this<ASTNode> {
    public:
        virtual ~ASTNode() = default;        
        virtual void print(std::string prefix = "", bool isLast = true) const = 0;
        virtual int transform(std::vector<Instruction>& program, int& tempCounter, 
            CompileContext&) const = 0;
        virtual std::shared_ptr<ASTNode> fold() {
            return shared_from_this();
        }
};

class NumberNode : public ASTNode {
    private:
        double value;
    public:
        NumberNode(double val) : value(val) {}
        int transform(std::vector<Instruction>& program, int& tempCounter, CompileContext& ctx) const override {
            if(ctx.consts.find(value) != ctx.consts.end()) {
                return ctx.consts[value];
            }
            
            int dest = tempCounter++;
            program.push_back({
                OpCode::LOAD_CONST, 0, 0, dest, value
            });

            ctx.consts[value] = dest;
            
            return dest;
        }
        void print(std::string prefix, bool isLast = true) const override {
            std::cout << prefix << (isLast ? "└── " : "├── ") << "Number: " << value << std::endl;
        }
        double getValue() const {
            return value;
        }
};

class VariableNode : public ASTNode {
    private:
        size_t address;
        const SymbolTable& symTable;
    public:
        VariableNode(size_t addr, const SymbolTable& st) : address(addr), symTable(st) {}
        int transform(std::vector<Instruction>& program, int& tempCounter, CompileContext& ctx) const override {
            if(ctx.vars.find(address) != ctx.vars.end()) {
                return ctx.vars[address];
            }
            int dest = tempCounter++;
            program.push_back({
                OpCode::LOAD_VAR, (int)address, 0, dest, 0.0
            });
            ctx.vars[address] = dest;
            return dest;
        }
        void print(std::string prefix, bool isLast) const override {
            std::cout << prefix << (isLast ? "└── " : "├── ") << "Var (Addr: " << address << ")" << std::endl;
        }
};

class BinaryOpNode : public ASTNode {
    private:
        std::string op;
        std::shared_ptr<ASTNode> left, right;
        OpCode getOpCode() const {
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
    public:
        BinaryOpNode(const std::string& o, std::shared_ptr<ASTNode> l, std::shared_ptr<ASTNode> r) :
            op(o), left(std::move(l)), right(std::move(r)) {}
        void print(std::string prefix, bool isLast) const override {
            std::cout << prefix << (isLast ? "└── " : "├── ") << "BinaryOp: " << op << std::endl;
            
            std::string newPrefix = prefix + (isLast ? "    " : "│   ");
            left -> print(newPrefix, false);
            right -> print(newPrefix, true);
        }
        int transform(std::vector<Instruction>& program, int& tempCounter, CompileContext& ctx) const override {
            int l_idx = left -> transform(program, tempCounter, ctx);
            int r_idx = right -> transform(program, tempCounter, ctx);

            int dest = tempCounter++;
            program.push_back({
                getOpCode(), l_idx, r_idx, dest, 0.0
            });
            return dest;
        }
        std::shared_ptr<ASTNode> fold() override {
            left = left -> fold();
            right = right -> fold();

            auto leftNum = std::dynamic_pointer_cast<NumberNode>(left);
            auto rightNum = std::dynamic_pointer_cast<NumberNode>(right);
            if(leftNum && rightNum) {
                double lVal = leftNum -> getValue();
                double rVal = rightNum -> getValue();
                double res = 0;
                if (op == "+") res = lVal + rVal;
                else if (op == "-") res = lVal - rVal;
                else if (op == "*") res = lVal * rVal;
                else if (op == "/") {
                    if (rVal != 0) res = lVal / rVal;
                    else res = 0;
                }
                else if(op == "&") {
                    long long l = static_cast<long long>(lVal);
                    long long r = static_cast<long long>(rVal);
                    res = static_cast<double>(l & r);
                }
                else if(op == "|") {
                    long long l = static_cast<long long>(lVal);
                    long long r = static_cast<long long>(rVal);
                    res = static_cast<double>(l | r);
                }
                else if(op == "^") {
                    long long l = static_cast<long long>(lVal);
                    long long r = static_cast<long long>(rVal);
                    res = static_cast<double>(l ^ r);
                }
                else if(op == "%") {
                    long long l = static_cast<long long>(lVal);
                    long long r = static_cast<long long>(rVal);
                    res = static_cast<double>(l % r);
                }
                else if(op == "<<") {
                    long long l = static_cast<long long>(lVal);
                    long long r = static_cast<long long>(rVal);
                    res = static_cast<double>(l << r);
                }
                else if(op == ">>") {
                    long long l = static_cast<long long>(lVal);
                    long long r = static_cast<long long>(rVal);
                    res = static_cast<double>(l >> r);
                }
                return std::make_shared<NumberNode>(res);
            }
            return shared_from_this();
        }
};

class UnaryOpNode : public ASTNode {
    private:
        std::string op; // '-' -> minus, '#' -> plus
        std::shared_ptr<ASTNode> child;
    public:
        UnaryOpNode(std::string& o, std::shared_ptr<ASTNode> c) : op(o), child(std::move(c)) {}
        UnaryOpNode(std::shared_ptr<ASTNode> c) : op("_"), child(std::move(c)) {}

        void print(std::string prefix, bool isLast) const override {
            std::string sign = (op == "-" || op == "_") ? "-" : "+";
            std::cout << prefix << (isLast ? "└── " : "├── ") << "UnaryOp: " << sign << std::endl;

            std::string newPrefix = prefix + (isLast ? "    " : "│   ");
            child -> print(newPrefix, true);
        }

        int transform(std::vector<Instruction>& program, int& tempCounter, CompileContext& ctx) const override {
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

        std::shared_ptr<ASTNode> fold() override {
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
};

