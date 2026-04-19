#pragma once
#include <vector>
#include <stack>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <string>
#include "ast.h"

struct Instruction {
    uint32_t op:    8;
    uint32_t dst:   8;
    uint32_t left:  8;
    uint32_t right: 8;
};

inline uint16_t getAddress(const Instruction& inst) {
    return (uint16_t)((inst.right << 8) | inst.left);
}
inline void setAddress(Instruction& inst, uint16_t addr) {
    inst.left = addr & 0xFF;
    inst.right = (addr >> 8) & 0xFF;
}

struct FunctionInfo {
    size_t address;
    int paramCount;
};

struct CompileContext {
    std::map<size_t, int> vars;
    std::map<double, int> consts;
};

struct ByteCode {
    std::vector<Instruction> instructions;
    std::vector<double> constants;
    std::vector<std::string> strings;
};

class Compiler {
    private:
        SymbolTable& symTable;
        int nextTempIndex = 0;
        CompileContext globalCtx;
        std::vector<double> constantPool;
        std::vector<std::string> stringPool;
        // Function Table
        std::unordered_map<std::string, FunctionInfo> functionTable;

        // Forward declarations
        std::vector<std::pair<size_t, std::string>> forwardCalls;
        int allocateTempRegister();

        std::vector<std::shared_ptr<ASTNode>> postOrderTraverse(std::shared_ptr<ASTNode> root);
        std::vector<Instruction> generateByteCode(const std::vector<std::shared_ptr<ASTNode>>& nodes);
public:
    Compiler(SymbolTable& st) : symTable(st) {}
    ByteCode compile(std::shared_ptr<ASTNode> root);
    void printByteCode(const std::vector<Instruction>& code) const;
    std::shared_ptr<ASTNode> optimize(std::shared_ptr<ASTNode> node);
    void compileStatement(std::shared_ptr<StatementNode> stmt, std::vector<Instruction>& code);
    const std::vector<std::string>& getStringPool() const {
        return stringPool;
    }
};

void writeByteCodeToFile(const ByteCode& bc, const std::string& path);
ByteCode readByteCodeFromFile(const std::string& path);