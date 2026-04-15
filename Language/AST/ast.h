#pragma once
#include <iostream>
#include <map>
#include <cstdint>
#include "../SymbolTable/symbol_table.h"

enum class OpCode : uint8_t {
    // RISC-V arithmetic (0-9)
    ADD = 0, SUB = 1, MUL = 2, DIV = 3, AND = 4, OR = 5, XOR = 6, MOD = 7, SLL = 8, SRL = 9,
    // Immediate (10-19)
    ADDI = 10, SUBI = 11, MULI = 12, DIVI = 13, ANDI = 14, ORI = 15, XORI = 16, MODI = 17, SLLI = 18, SRLI = 19,
    // Load/Store (20-21)
    LW = 20, SW = 21,
    // Branches (22-25)
    BEQ = 22, BNE = 23, BLT = 24, BGE = 25,
    // Jumps (26-27)
    JAL = 26, JALR = 27,
    // Extended (28+)
    LOAD_CONST = 28, LOAD_STR = 29, PRINT = 30, PRINT_STR = 31,
    // Comparisons (32-37)
    CMP_EQ = 32, CMP_NE = 33, CMP_LT = 34, CMP_GT = 35, CMP_LE = 36, CMP_GE = 37,
    // Logical (38-40)
    LOGICAL_AND = 38, LOGICAL_OR = 39, LOGICAL_NOT = 40,
    HALT = 41, POW = 42,
    UNDEFINED = 255
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(std::string prefix = "", bool isLast = true) const = 0;
    virtual std::vector<std::shared_ptr<ASTNode>> getChildren() const = 0;
};

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double val) : value(val) {}
    double getValue() const { return value; }
    void print(std::string prefix, bool isLast) const override;
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};

class VariableNode : public ASTNode {
    size_t offset;   // offset from frame pointer
public:
    VariableNode(size_t off) : offset(off) {}
    size_t getOffset() const { return offset; }
    void print(std::string prefix, bool isLast) const override;
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};

class BinaryOpNode : public ASTNode {
    std::string op;
    std::shared_ptr<ASTNode> left, right;
public:
    BinaryOpNode(const std::string& o, std::shared_ptr<ASTNode> l, std::shared_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
    void print(std::string prefix, bool isLast) const override;
    OpCode getOpCode() const;  // returns appropriate OpCode
    std::shared_ptr<ASTNode> getLeft() const { return left; }
    std::shared_ptr<ASTNode> getRight() const { return right; }
    std::string getOp() const { return op; }
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {left, right}; }
};

class UnaryOpNode : public ASTNode {
    std::string op;
    std::shared_ptr<ASTNode> child;
public:
    UnaryOpNode(const std::string& o, std::shared_ptr<ASTNode> c) : op(o), child(std::move(c)) {}
    void print(std::string prefix, bool isLast) const override;
    std::string getOp() const { return op; }
    std::shared_ptr<ASTNode> getChild() const { return child; }
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {child}; }
};

class StatementNode : public ASTNode {
public:
    virtual ~StatementNode() = default;
    virtual void print(std::string prefix, bool isLast) const = 0;
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};

class BlockCode : public StatementNode {
    std::vector<std::shared_ptr<StatementNode>> statements;
public:
    void addStatement(std::shared_ptr<StatementNode> stmt) { statements.push_back(stmt); }
    void print(std::string prefix, bool isLast) const override;
    std::vector<std::shared_ptr<StatementNode>> getStatements() const { return statements; }
};

class AssignmentNode : public StatementNode {
    size_t offset;
    std::shared_ptr<ASTNode> expression;
public:
    AssignmentNode(size_t off, std::shared_ptr<ASTNode> expr) : offset(off), expression(std::move(expr)) {}
    void print(std::string prefix, bool isLast) const override;
    std::shared_ptr<ASTNode> getExpression() const { return expression; }
    size_t getOffset() const { return offset; }
};

class IfStatementNode : public StatementNode {
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<StatementNode> thenBranch, elseBranch;
public:
    IfStatementNode(std::shared_ptr<ASTNode> cond, std::shared_ptr<StatementNode> thenBr,
                    std::shared_ptr<StatementNode> elseBr = nullptr)
        : condition(cond), thenBranch(thenBr), elseBranch(elseBr) {}
    void print(std::string prefix, bool isLast) const override;
    std::shared_ptr<ASTNode> getCondition() const { return condition; }
    std::shared_ptr<StatementNode> getThenBr() const { return thenBranch; }
    std::shared_ptr<StatementNode> getElseBr() const { return elseBranch; }
};

class WhileStatementNode : public StatementNode {
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<StatementNode> body;
public:
    WhileStatementNode(std::shared_ptr<ASTNode> cond, std::shared_ptr<StatementNode> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    void print(std::string prefix, bool isLast) const override;
    std::shared_ptr<ASTNode> getCondition() const { return condition; }
    std::shared_ptr<StatementNode> getBody() const { return body; }
};

class PrintNode : public StatementNode {
    std::vector<std::shared_ptr<ASTNode>> expressions;
public:
    PrintNode(std::vector<std::shared_ptr<ASTNode>> exprs) : expressions(std::move(exprs)) {}
    const std::vector<std::shared_ptr<ASTNode>>& getExpressions() const { return expressions; }
    void print(std::string prefix, bool isLast) const override;
};

class ForStatementNode : public StatementNode {
    std::shared_ptr<StatementNode> init;
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<StatementNode> update;
    std::shared_ptr<StatementNode> body;
public:
    ForStatementNode(std::shared_ptr<StatementNode> in, std::shared_ptr<ASTNode> cond,
                     std::shared_ptr<StatementNode> updt, std::shared_ptr<StatementNode> bdy)
        : init(std::move(in)), condition(std::move(cond)), update(std::move(updt)), body(std::move(bdy)) {}
    void print(std::string prefix, bool isLast) const override;
    std::shared_ptr<StatementNode> getInit() const { return init; }
    std::shared_ptr<ASTNode> getCondition() const { return condition; }
    std::shared_ptr<StatementNode> getUpdate() const { return update; }
    std::shared_ptr<StatementNode> getBody() const { return body; }
};

class StringNode : public ASTNode {
    std::string value;
public:
    StringNode(const std::string& val = "") : value(val) {}
    const std::string& getValue() const { return value; }
    void print(std::string prefix, bool isLast) const override;
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};

class FunctionDefNode : public StatementNode {
    std::string name;
    std::vector<std::string> params;
    std::shared_ptr<StatementNode> body;
public:
    FunctionDefNode(const std::string& n, std::vector<std::string> p, std::shared_ptr<StatementNode> b)
        : name(n), params(std::move(p)), body(std::move(b)) {}
    const std::string& getName() const { return name; }
    const std::vector<std::string>& getParams() const { return params; }
    std::shared_ptr<StatementNode> getBody() const { return body; }
    void print(std::string prefix, bool isLast) const override;
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};

class FunctionCallNode : public ASTNode {
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> args;
public:
    FunctionCallNode(const std::string& n, std::vector<std::shared_ptr<ASTNode>> a)
        : name(n), args(std::move(a)) {}
    const std::string& getName() const { return name; }
    const std::vector<std::shared_ptr<ASTNode>>& getArgs() const { return args; }
    void print(std::string prefix, bool isLast) const override;
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};

class ReturnNode : public StatementNode {
    std::shared_ptr<ASTNode> expression;
public:
    ReturnNode(std::shared_ptr<ASTNode> expr) : expression(std::move(expr)) {}
    std::shared_ptr<ASTNode> getExpression() const { return expression; }
    void print(std::string prefix, bool isLast) const override;
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};

class FunctionCallStatementNode : public StatementNode {
    std::shared_ptr<FunctionCallNode> call;
public:
    FunctionCallStatementNode(std::shared_ptr<ASTNode> c)
        : call(std::dynamic_pointer_cast<FunctionCallNode>(c)) {}
    std::shared_ptr<FunctionCallNode> getCall() const { return call; }
    void print(std::string prefix, bool isLast) const override;
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};