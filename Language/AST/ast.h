#pragma once
#include <iostream>
#include <map>
#include <cstdint>
#include "../SymbolTable/symbol_table.h"

enum class OpCode : uint8_t {
    ADD, SUB, MUL, DIV, AND, OR, XOR, MODULO,
    LSHIFT, RSHIFT, UNARY, LOAD_CONST, LOAD_VAR, LOAD_STR,
    UNDEFINED,
    CMP_GT, CMP_LT, CMP_GET, CMP_LET, CMP_EQ, CMP_NEQ,
    JMP, JZ, JNZ,
    STORE_VAR,
    PRINT, PRINT_STR,
    LOGICAL_AND, LOGICAL_OR, LOGICAL_NOT,
    CALL, RETURN, PUSH_ARG, LOAD_PARAM,
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
    size_t address;
public:
    VariableNode(size_t addr) : address(addr) {}
    void print(std::string prefix, bool isLast) const override;
    size_t get_address() const { return address; }
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};

class BinaryOpNode : public ASTNode {
    std::string op;
    std::shared_ptr<ASTNode> left, right;
public:
    BinaryOpNode(const std::string& o, std::shared_ptr<ASTNode> l, std::shared_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
    void print(std::string prefix, bool isLast) const override;
    OpCode getOpCode() const;
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
    size_t address;
    std::shared_ptr<ASTNode> expression;
public:
    AssignmentNode(size_t addr, std::shared_ptr<ASTNode> expr)
        : address(addr), expression(std::move(expr)) {}
    void print(std::string prefix, bool isLast) const override;
    std::shared_ptr<ASTNode> getExpression() const { return expression; }
    size_t getAddress() const { return address; }
};

class IfStatementNode : public StatementNode {
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<StatementNode> thenBranch, elseBranch;
public:
    IfStatementNode(std::shared_ptr<ASTNode> cond,
                    std::shared_ptr<StatementNode> thenBr,
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
    private:
        std::shared_ptr<StatementNode> init; // i = start
        std::shared_ptr<ASTNode> condition; // i < 10
        std::shared_ptr<StatementNode> update; // i = i+1
        std::shared_ptr<StatementNode> body; // i = i+1
    public:
        ForStatementNode(std::shared_ptr<StatementNode> in, 
                         std::shared_ptr<ASTNode> cond, 
                         std::shared_ptr<StatementNode> updt,
                         std::shared_ptr<StatementNode> bdy)
        : init(std::move(in)), condition(std::move(cond)), update(std::move(updt)), body(std::move(bdy)) {}
        void print(std::string prefix, bool isLast) const override;
        std::shared_ptr<StatementNode> getInit()      const { return init; }
    std::shared_ptr<ASTNode>       getCondition() const { return condition; }
    std::shared_ptr<StatementNode> getUpdate()    const { return update; }
    std::shared_ptr<StatementNode> getBody()      const { return body; }
};

class StringNode : public ASTNode {
    private:
        std::string value;
    public:
        StringNode(const std::string& val = "") : value(val) {}
        const std::string getValue() const {
            return value;
        }
        void print(std::string prefix, bool isLast) const override {
            std::cout << prefix << (isLast ? "└── " : "├── ") << "String: \"" << value << "\"" << std::endl;
        }
        std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};

// function definition
class FunctionDefNode : public StatementNode {
    private:
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

// function call
class FunctionCallNode : public ASTNode {
    private:
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

// return statement
class ReturnNode : public StatementNode {
    private:
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
    void print(std::string prefix, bool isLast) const override {
        call->print(prefix, isLast);
    }
    std::vector<std::shared_ptr<ASTNode>> getChildren() const override { return {}; }
};