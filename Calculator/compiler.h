#pragma once
#include <vector>
#include <stack>
#include <memory>
#include <algorithm>
#include "ast.h"

// enum class OpCode {
//     ADD, SUB, MUL, DIV, AND, OR, XOR, MODULO, 
//     LSHIFT, RSHIFT, UNARY, LOAD_CONST, LOAD_VAR,
//     UNDEFINED,
// };

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

class Compiler {
    private:
        int nextTempIndex = 0;
        std::vector<std::shared_ptr<ASTNode>> postOrderTraverse(std::shared_ptr<ASTNode> root);
        std::vector<Instruction> generateByteCode(const std::vector<std::shared_ptr<ASTNode>>& nodes);
    public:
        std::vector<Instruction> compile(std::shared_ptr<ASTNode> root);
        void printByteCode(const std::vector<Instruction>& code) const;
};