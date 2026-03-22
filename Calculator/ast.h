#pragma once
#include <iostream>
#include <map>
#include "symbol_table.h"

enum class OpCode {
    ADD, SUB, MUL, DIV, AND, OR, XOR, MODULO, 
    LSHIFT, RSHIFT, UNARY, LOAD_CONST, LOAD_VAR,
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
        int transform(std::vector<Instruction>& program, int& tempCounter, CompileContext& ctx) const override;
        void print(std::string prefix, bool isLast = true) const override;
        double getValue() const;
};

class VariableNode : public ASTNode {
    private:
        size_t address;
        const SymbolTable& symTable;
    public:
        VariableNode(size_t addr, const SymbolTable& st) : address(addr), symTable(st) {}
        int transform(std::vector<Instruction>& program, int& tempCounter, CompileContext& ctx) const override;
        void print(std::string prefix, bool isLast) const;
};

class BinaryOpNode : public ASTNode {
    private:
        std::string op;
        std::shared_ptr<ASTNode> left, right;
        OpCode getOpCode() const;
    public:
        BinaryOpNode(const std::string& o, std::shared_ptr<ASTNode> l, std::shared_ptr<ASTNode> r) :
            op(o), left(std::move(l)), right(std::move(r)) {}
        void print(std::string prefix, bool isLast) const override;
        int transform(std::vector<Instruction>& program, int& tempCounter, CompileContext& ctx) const override;
        std::shared_ptr<ASTNode> fold() override;
};

class UnaryOpNode : public ASTNode {
    private:
        std::string op; // '-' -> minus, '#' -> plus
        std::shared_ptr<ASTNode> child;
    public:
        UnaryOpNode(std::string& o, std::shared_ptr<ASTNode> c) : op(o), child(std::move(c)) {}
        UnaryOpNode(std::shared_ptr<ASTNode> c) : op("_"), child(std::move(c)) {}

        void print(std::string prefix, bool isLast) const override;
        int transform(std::vector<Instruction>& program, int& tempCounter, CompileContext& ctx) const override;
        std::shared_ptr<ASTNode> fold() override;
};

