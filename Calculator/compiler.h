#pragma once
#include <vector>
#include <stack>
#include <memory>
#include <algorithm>
#include <cstdint> // uint_32t
#include "ast.h" // for OpCode enum class

struct Instruction {
    uint32_t op:    8; // OpCode
    uint32_t dst:   8; // Destination
    uint32_t left:  8; // Left Operand / Constant index
    uint32_t right: 8; // Right Operand
};

// static_assert(sizeof(Instruction) == 4, "Instruction must be exactly 4 bytes");

struct CompileContext {
    std::map<size_t, int> vars;
    std::map<double, int> consts;
};

struct ByteCode {
    std::vector<Instruction> instructions;
    std::vector<double> constants;
};

class Compiler {
    private:
        int nextTempIndex = 0;
        std::vector<double> constantPool;
        std::vector<std::shared_ptr<ASTNode>> postOrderTraverse(std::shared_ptr<ASTNode> root);
        std::vector<Instruction> generateByteCode(const std::vector<std::shared_ptr<ASTNode>>& nodes);
        public:
        ByteCode compile(std::shared_ptr<ASTNode> root);
        void printByteCode(const std::vector<Instruction>& code) const;
        std::shared_ptr<ASTNode> optimize(std::shared_ptr<ASTNode> node);
};