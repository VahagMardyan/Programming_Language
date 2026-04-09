#pragma once
#include <vector>
#include <stack>
#include <memory>
#include <algorithm>
#include <cstdint>
#include "ast.h"

struct Instruction {
    uint32_t op:    8;
    uint32_t dst:   8;
    uint32_t left:  8;
    uint32_t right: 8;
};

struct CompileContext {
    std::map<size_t, int> vars;
    std::map<double, int> consts;
};

struct ByteCode {
    std::vector<Instruction> instructions;
    std::vector<double> constants;
};

class Compiler {
    int nextTempIndex = 0;
    CompileContext globalCtx;
    std::vector<double> constantPool;
    std::vector<std::shared_ptr<ASTNode>> postOrderTraverse(std::shared_ptr<ASTNode> root);
    std::vector<Instruction> generateByteCode(const std::vector<std::shared_ptr<ASTNode>>& nodes);
public:
    ByteCode compile(std::shared_ptr<ASTNode> root);
    void printByteCode(const std::vector<Instruction>& code) const;
    std::shared_ptr<ASTNode> optimize(std::shared_ptr<ASTNode> node);
    void compileStatement(std::shared_ptr<StatementNode> stmt, std::vector<Instruction>& code);
};