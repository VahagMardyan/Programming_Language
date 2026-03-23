#pragma once
#include <iostream>
#include <map>
#include "symbol_table.h"

enum class OpCode {
    ADD, SUB, MUL, DIV, AND, OR, XOR, MODULO, 
    LSHIFT, RSHIFT, UNARY, LOAD_CONST, LOAD_VAR,
    UNDEFINED,
};

class ASTNode : public std::enable_shared_from_this<ASTNode> {
    public:
        virtual ~ASTNode() = default;        
        virtual void print(std::string prefix = "", bool isLast = true) const = 0;
        virtual std::vector<std::shared_ptr<ASTNode>> getChildren() const = 0;
};

class NumberNode : public ASTNode {
    private:
        double value;
    public:
        NumberNode(double val) : value(val) {}
        double getValue() const {
            return value;
        };
        void print(std::string prefix, bool isLast) const override;
        virtual std::vector<std::shared_ptr<ASTNode>> getChildren() const override {
            return {};
        }
};

class VariableNode : public ASTNode {
    private:
        size_t address;
    public:
        VariableNode(size_t addr) : address(addr){}
        void print(std::string prefix, bool isLast) const override;
        size_t get_address() const {
            return address;
        }
        std::vector<std::shared_ptr<ASTNode>> getChildren() const override {
            return {};
        }
};

class BinaryOpNode : public ASTNode {
    private:
        std::string op;
        std::shared_ptr<ASTNode> left, right;
    public:
        BinaryOpNode(const std::string& o, std::shared_ptr<ASTNode> l, std::shared_ptr<ASTNode> r) :
        op(o), left(std::move(l)), right(std::move(r)) {}
        void print(std::string prefix, bool isLast) const override;
        OpCode getOpCode() const;
        std::string getOp() const {
            return op;
        }
        std::vector<std::shared_ptr<ASTNode>> getChildren() const override {
            return {left, right};
        }
};

class UnaryOpNode : public ASTNode {
    private:
        std::string op; // '-' -> minus, '#' -> plus
        std::shared_ptr<ASTNode> child;
    public:
        UnaryOpNode(const std::string& o, std::shared_ptr<ASTNode> c) : op(o), child(std::move(c)) {}
        UnaryOpNode(std::shared_ptr<ASTNode> c) : op("_"), child(std::move(c)) {}

        void print(std::string prefix, bool isLast) const override;

        std::string getOp() const {
            return op;
        }
        
        std::vector<std::shared_ptr<ASTNode>> getChildren() const override {
            return {child};
        }
};

