#pragma once
#include <vector>
#include <stack>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include "../AST/ast.h"

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
    std::vector<int> lineNumbers;
    std::unordered_map<std::string, size_t> functionSymbols;
    std::vector<std::pair<size_t, std::string>> unresolvedCalls;
    /** Globals slots 0 .. globalSlotCount-1; names for runtime errors (LOAD before STORE). */
    size_t globalSlotCount = 0;
    std::vector<std::string> globalNamesBySlot;
};

class Compiler {
    private:
        SymbolTable& symTable;
        int nextTempIndex = 0;
        std::stack<int> freeRegisters;
        CompileContext globalCtx;
        std::vector<double> constantPool;
        std::vector<std::string> stringPool;
        std::unordered_map<std::string, int> stringMap;
        std::unordered_map<double, int> constMap;
        // Function Table
        std::unordered_map<std::string, FunctionInfo> functionTable;

        // Forward declarations
        std::vector<std::pair<size_t, std::string>> forwardCalls;
        int allocateTempRegister();
        void freeTempRegister(int reg);
        void emitMainPrologue(std::vector<Instruction>& code);

        std::vector<std::shared_ptr<ASTNode>> postOrderTraverse(std::shared_ptr<ASTNode> root);
        std::vector<Instruction> generateByteCode(const std::vector<std::shared_ptr<ASTNode>>& nodes);
        bool tryEmitMathBuiltinCall(
            const std::string& name,
            const std::vector<std::shared_ptr<ASTNode>>& args,
            std::vector<Instruction>& code,
            int& resultReg
        );
        std::stack<std::vector<size_t>> breakStack;
        std::stack<std::vector<size_t>> continueStack;

        std::vector<int> lineNumbers;
        void addLineNumbers(int line, size_t count) {
            lineNumbers.insert(lineNumbers.end(), count, line);
        }
public:
    Compiler(SymbolTable& st) : symTable(st) {}
    ByteCode compile(
        std::shared_ptr<ASTNode> root,
        bool allowUnresolvedCalls = false,
        bool emitMainFramePrologue = true
    );
    void printByteCode(const std::vector<Instruction>& code) const;
    std::shared_ptr<ASTNode> optimize(std::shared_ptr<ASTNode> node);
    void compileStatement(std::shared_ptr<StatementNode> stmt, std::vector<Instruction>& code);
    const std::vector<std::string>& getStringPool() const {
        return stringPool;
    }
};

void writeByteCodeToFile(const ByteCode& bc, const std::string& path);
ByteCode readByteCodeFromFile(const std::string& path);