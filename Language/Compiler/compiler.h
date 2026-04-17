#pragma once
#include <vector>
#include <stack>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include "ast.h"
#include "../SymbolTable/symbol_table.h"

struct Instruction {
    uint32_t op:8;
    uint32_t rd:8;
    uint32_t rs1:8;
    uint32_t rs2:8;
};

inline uint16_t getImmediate(const Instruction& inst) {
    return (uint16_t)((inst.rs2 << 8) | inst.rs1);
}

inline void setImmediate(Instruction& inst, uint16_t imm) {
    inst.rs1 = imm & 0xFF;
    inst.rs2 = (imm >> 8) & 0xFF;
}

struct FunctionInfo {
    size_t address;
    int paramCount;
    int frameSize;
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
    std::unordered_map<std::string, int> regMap;
    std::unordered_map<double, int> constMap;
    int nextReg = 5;
};

struct ByteCode {
    std::vector<Instruction> instructions;
    std::vector<double> constants;
    std::vector<std::string> strings;
};

class Compiler {
<<<<<<< HEAD
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
=======
private:
    SymbolTable* symTable;
    std::vector<double> constantPool;
    std::vector<std::string> stringPool;
    std::unordered_map<std::string, FunctionInfo> functionTable;
    std::vector<std::pair<size_t, std::string>> forwardCalls;
    int nextLabel = 0;

    std::vector<std::shared_ptr<ASTNode>> postOrderTraverse(std::shared_ptr<ASTNode> root);
    std::vector<Instruction> generateByteCode(const std::vector<std::shared_ptr<ASTNode>>& nodes, CompileContext& ctx);
    void compileStatement(std::shared_ptr<StatementNode> stmt, std::vector<Instruction>& code, CompileContext& ctx);
    int newLabel() { return nextLabel++; }

public:
    Compiler(SymbolTable* st) : symTable(st) {}
    ByteCode compile(std::shared_ptr<ASTNode> root);
    void printByteCode(const std::vector<Instruction>& code) const;
    std::shared_ptr<ASTNode> optimize(std::shared_ptr<ASTNode> node);
    const std::vector<std::string>& getStringPool() const { return stringPool; }
>>>>>>> 37c62253fa08934c2bae054db3a95e11c543af6e
};