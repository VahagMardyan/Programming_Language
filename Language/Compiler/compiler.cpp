#include "compiler.h"
#include <cmath>
#include <fstream>
#include <cstring>

const int SP = 2;
const int FP = 8;

int Compiler::allocateTempRegister() {
    while(nextTempIndex == SP || nextTempIndex == FP) {
        ++nextTempIndex;
    }
    return nextTempIndex++;
}

std::vector<std::shared_ptr<ASTNode>> Compiler::postOrderTraverse(std::shared_ptr<ASTNode> root) {
    if(!root) return {};
    std::vector<std::shared_ptr<ASTNode>> postOrder;
    std::stack<std::shared_ptr<ASTNode>> s1, s2;
    s1.push(root);
    while(!s1.empty()) {
        auto node = s1.top(); s1.pop(); s2.push(node);
        for(auto& child : node->getChildren()) s1.push(child);
    }
    while(!s2.empty()) { postOrder.push_back(s2.top()); s2.pop(); }
    return postOrder;
}

std::shared_ptr<ASTNode> Compiler::optimize(std::shared_ptr<ASTNode> node) {
    if(!node) return nullptr;
    if(auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        auto left  = optimize(bin->getLeft());
        auto right = optimize(bin->getRight());
        auto lNum  = std::dynamic_pointer_cast<NumberNode>(left);
        auto rNum  = std::dynamic_pointer_cast<NumberNode>(right);
        if(lNum && rNum) {
            double v1 = lNum->getValue(), v2 = rNum->getValue(), result = 0;
            switch(bin->getOpCode()) {
                case OpCode::ADD: result = v1+v2; break;
                case OpCode::SUB: result = v1-v2; break;
                case OpCode::MUL: result = v1*v2; break;
                case OpCode::POW: result = std::pow(v1, v2); break;
                case OpCode::DIV: result = v2 ? v1/v2 : 0; break;
                case OpCode::AND: result = (double)((long long)v1 & (long long)v2); break;
                case OpCode::OR:  result = (double)((long long)v1 | (long long)v2); break;
                case OpCode::XOR: result = (double)((long long)v1 ^ (long long)v2); break;
                case OpCode::MODULO: result = (double)((long long)v1 % (long long)v2); break;
                case OpCode::SLL: result = (double)((long long)v1 << (long long)v2); break;
                case OpCode::SRL: result = (double)((long long)v1 >> (long long)v2); break;
                case OpCode::LOGICAL_AND: result = (v1 != 0 && v2 != 0) ? 1.0 : 0.0; break;
                case OpCode::LOGICAL_OR: result = (v1 != 0 || v2 != 0) ? 1.0 : 0.0; break;
                case OpCode::SLT: result = (v1 < v2) ? 1.0 : 0.0; break;
                case OpCode::CMP_LET: result = (v1 <= v2) ? 1.0 : 0.0; break;
                case OpCode::CMP_GT: result = (v1 > v2) ? 1.0 : 0.0; break;
                case OpCode::CMP_GET: result = (v1 >= v2) ? 1.0 : 0.0; break;
                case OpCode::CMP_EQ: result = (v1 == v2) ? 1.0 : 0.0; break;
                case OpCode::CMP_NEQ: result = (v1 != v2) ? 1.0 : 0.0; break;
                default: break;
            }
            return std::make_shared<NumberNode>(result);
        }
        return std::make_shared<BinaryOpNode>(bin->getOp(), left, right);
    }
    if(auto block = std::dynamic_pointer_cast<BlockCode>(node)) {
        auto optimizedBlock = std::make_shared<BlockCode>();
        for(auto& s : block->getStatements()) {
            auto optStmt = std::dynamic_pointer_cast<StatementNode>(optimize(s));
            if(optStmt) optimizedBlock->addStatement(optStmt);
        }
        return optimizedBlock;
    }
    if(auto assign = std::dynamic_pointer_cast<AssignmentNode>(node)) {
        if (assign->isLocal()) {
            return std::make_shared<AssignmentNode>(assign->getOffset(), optimize(assign->getValue()));
        } else {
            return std::make_shared<AssignmentNode>(assign->getAddress(), optimize(assign->getValue()));
        }
    }
    if(auto ifStmt = std::dynamic_pointer_cast<IfStatementNode>(node)) {
        auto cond   = optimize(ifStmt->getCondition());
        auto thenBr = std::dynamic_pointer_cast<StatementNode>(optimize(ifStmt->getThenBr()));
        auto elseBr = std::dynamic_pointer_cast<StatementNode>(optimize(ifStmt->getElseBr()));
        if(auto condNum = std::dynamic_pointer_cast<NumberNode>(cond))
            return condNum->getValue() != 0 ? thenBr : (elseBr ? elseBr : nullptr);
        return std::make_shared<IfStatementNode>(cond, thenBr, elseBr);
    }
    if(auto whileStmt = std::dynamic_pointer_cast<WhileStatementNode>(node)) {
        auto cond = optimize(whileStmt->getCondition());
        auto body = std::dynamic_pointer_cast<StatementNode>(optimize(whileStmt->getBody()));
        if(auto condNum = std::dynamic_pointer_cast<NumberNode>(cond))
            if(condNum->getValue() == 0) return nullptr;
        return std::make_shared<WhileStatementNode>(cond, body);
    }
    if(auto printStmt = std::dynamic_pointer_cast<PrintNode>(node)) {
        std::vector<std::shared_ptr<ASTNode>> exprs;
        for(const auto& e : printStmt->getExpressions()) exprs.push_back(optimize(e));
        return std::make_shared<PrintNode>(std::move(exprs));
    }
    if(auto forStmt = std::dynamic_pointer_cast<ForStatementNode>(node)) {
        auto init = std::dynamic_pointer_cast<StatementNode>(optimize(forStmt -> getInit()));
        auto cond = optimize(forStmt -> getCondition());
        auto update = std::dynamic_pointer_cast<StatementNode>(optimize(forStmt -> getUpdate()));
        auto body = std::dynamic_pointer_cast<StatementNode>(optimize(forStmt -> getBody()));
        return std::make_shared<ForStatementNode>(init, cond, update, body);
    }
    if(auto strNode = std::dynamic_pointer_cast<StringNode>(node)) {
        return strNode;
    }
    return node;
}

ByteCode Compiler::compile(std::shared_ptr<ASTNode> root) {
    constantPool.clear();
    stringPool.clear();
    nextTempIndex = 0;
    functionTable.clear();
    forwardCalls.clear();
    
    std::vector<Instruction> insts;
    if(!root) return {insts, constantPool, stringPool};
    
    auto optimizedRoot = optimize(root);
    auto stmt = std::dynamic_pointer_cast<StatementNode>(optimizedRoot);
    if(stmt) compileStatement(stmt, insts);
    
    for(auto& [idx, name] : forwardCalls) {
        if(functionTable.count(name)) {
            setAddress(insts[idx], (uint16_t)functionTable[name].address);
        } else {
            throw std::runtime_error("Undefined function: " + name);
        }
    }    
    return {insts, constantPool, stringPool};
}

std::vector<Instruction> Compiler::generateByteCode(const std::vector<std::shared_ptr<ASTNode>>& nodes) {
    std::vector<Instruction> code;
    std::stack<int> storage;
    for(const auto& node : nodes) {
        if(auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
            double val = num->getValue();
            if(globalCtx.consts.find(val) == globalCtx.consts.end()) {
                int reg = allocateTempRegister(), idx = (int)constantPool.size();
                constantPool.push_back(val);
                code.push_back({(uint32_t)OpCode::LOAD_CONST, (uint32_t)reg, (uint32_t)idx, 0});
                globalCtx.consts[val] = reg;
            }
            storage.push(globalCtx.consts[val]);        
        } else if(auto var = std::dynamic_pointer_cast<VariableNode>(node)) {
            int rd = allocateTempRegister();
            if (var->getIsLocal()) {
                int32_t off = var->getLocalOffset();
                code.push_back({(uint32_t)OpCode::LOAD, (uint32_t)rd, (uint32_t)FP, (uint32_t)off});
            } else {
                size_t addr = var->getGlobalAddr();
                code.push_back({(uint32_t)OpCode::LOAD_VAR, (uint32_t)rd, (uint32_t)addr, 0});
            }
            storage.push(rd);
        }
        else if(auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
            int r = storage.top(); storage.pop();
            int l = storage.top(); storage.pop();
            int target = allocateTempRegister();
            code.push_back({(uint32_t)bin->getOpCode(), (uint32_t)target, (uint32_t)l, (uint32_t)r});
            storage.push(target);
        } else if(auto un = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
            int childIdx = storage.top(); storage.pop();
            int target = allocateTempRegister();
            OpCode opcode = (un -> getOp() == "not") ? OpCode::LOGICAL_NOT : OpCode::UNARY;
            code.push_back({(uint32_t)opcode, (uint32_t)target, (uint32_t)childIdx, 0});
            storage.push(target);
        } else if(auto strNode = std::dynamic_pointer_cast<StringNode>(node)) {
            int strIdx = (int)stringPool.size();
            stringPool.push_back(strNode -> getValue());
            int reg = allocateTempRegister();
            code.push_back({
                (uint32_t)OpCode::LOAD_STR, (uint32_t)reg, (uint32_t)strIdx, 0
            });
            storage.push(reg);
        } else if(auto callExpr = std::dynamic_pointer_cast<FunctionCallNode>(node)) {
            for(const auto& arg : callExpr->getArgs()) {
                globalCtx.consts.clear();
                globalCtx.vars.clear();
                auto argCode = generateByteCode(postOrderTraverse(arg));
                code.insert(code.end(), argCode.begin(), argCode.end());
                int argReg = argCode.empty() ? 0 : argCode.back().dst;
                code.push_back({(uint32_t)OpCode::PUSH_ARG, (uint32_t)argReg, 0, 0});
            }
        
            int resultReg = allocateTempRegister();
            Instruction callInst;
            callInst.op  = (uint32_t)OpCode::CALL;
            callInst.dst = (uint32_t)resultReg;
            if(functionTable.count(callExpr->getName())) {
                setAddress(callInst, (uint16_t)functionTable[callExpr->getName()].address);
            } else {
                setAddress(callInst, 0);
                forwardCalls.push_back({code.size(), callExpr->getName()});
            }
            code.push_back(callInst);
            storage.push(resultReg);
        }  
    }
    return code;
}

void Compiler::compileStatement(std::shared_ptr<StatementNode> stmt, std::vector<Instruction>& code) {
    if(!stmt) return;
    
    if (auto assign = std::dynamic_pointer_cast<AssignmentNode>(stmt)) {
        globalCtx.consts.clear();
        globalCtx.vars.clear();
        auto exprCode = generateByteCode(postOrderTraverse(assign->getValue()));
        code.insert(code.end(), exprCode.begin(), exprCode.end());
        
        int srcReg = exprCode.empty() ? 0 : exprCode.back().dst;
        
        if (assign->isLocal()) {
            int32_t offset = assign->getOffset();
            code.push_back({(uint32_t)OpCode::STORE, 
                            (uint32_t)srcReg,
                            (uint32_t)FP,
                            (uint32_t)offset});
        } 
        else {
            size_t addr = assign->getAddress();
            code.push_back({(uint32_t)OpCode::STORE_VAR, 
                            0,
                            (uint32_t)addr,
                            (uint32_t)srcReg});
        }
    }
    
    else if(auto ifStmt = std::dynamic_pointer_cast<IfStatementNode>(stmt)) {
        globalCtx.consts.clear(); globalCtx.vars.clear();
        auto condCode = generateByteCode(postOrderTraverse(ifStmt->getCondition()));
        code.insert(code.end(), condCode.begin(), condCode.end());
        int condReg = condCode.empty() ? 0 : condCode.back().dst;
        size_t jzIdx = code.size();
        code.push_back({(uint32_t)OpCode::JZ, (uint32_t)condReg, 0, 0});
        compileStatement(ifStmt->getThenBr(), code);
        size_t jmpIdx = code.size();
        code.push_back({(uint32_t)OpCode::JMP, 0, 0, 0});
        setAddress(code[jzIdx], (uint16_t)code.size());
        if(ifStmt->getElseBr()) compileStatement(ifStmt->getElseBr(), code);
        setAddress(code[jmpIdx], (uint16_t)code.size());
    } else if(auto whileStmt = std::dynamic_pointer_cast<WhileStatementNode>(stmt)) {
        size_t startAddr = code.size();
        globalCtx.consts.clear(); globalCtx.vars.clear();
        auto condCode = generateByteCode(postOrderTraverse(whileStmt->getCondition()));
        code.insert(code.end(), condCode.begin(), condCode.end());
        int condReg = condCode.empty() ? 0 : condCode.back().dst;
        size_t jzIdx = code.size();
        code.push_back({(uint32_t)OpCode::JZ, (uint32_t)condReg, 0, 0});
        compileStatement(whileStmt->getBody(), code);
        Instruction jmpBack = {(uint32_t)OpCode::JMP, 0, 0, 0};
        setAddress(jmpBack, (uint16_t)startAddr);
        code.push_back(jmpBack);
        setAddress(code[jzIdx], (uint16_t)code.size());
    } else if(auto block = std::dynamic_pointer_cast<BlockCode>(stmt)) {
        for(auto& s : block->getStatements()) compileStatement(s, code);
    } else if(auto printStmt = std::dynamic_pointer_cast<PrintNode>(stmt)) {
        for(const auto& expr : printStmt->getExpressions()) {
            // String Literal
            if(auto strNode = std::dynamic_pointer_cast<StringNode>(expr)) {
                int strIdx = (int)stringPool.size();
                stringPool.push_back(strNode -> getValue());
                code.push_back({(uint32_t)OpCode::PRINT_STR, (uint32_t)strIdx, 0, 0});
            } else {
                // Number / expression
                globalCtx.consts.clear(); globalCtx.vars.clear();
                auto exprCode = generateByteCode(postOrderTraverse(expr));
                code.insert(code.end(), exprCode.begin(), exprCode.end());
                int lastReg = exprCode.empty() ? 0 : exprCode.back().dst;
                code.push_back({(uint32_t)OpCode::PRINT, (uint32_t)lastReg, 0, 0});
            }
        }
    } else if(auto forStmt = std::dynamic_pointer_cast<ForStatementNode>(stmt)) {
        // init
        compileStatement(forStmt -> getInit(), code);

        // loop start
        size_t startAddr = code.size();
        globalCtx.consts.clear();
        globalCtx.vars.clear();

        // condition
        auto condCode = generateByteCode(postOrderTraverse(forStmt -> getCondition()));
        code.insert(code.end(), condCode.begin(), condCode.end());
        int condReg = condCode.empty() ? 0 : condCode.back().dst;

        size_t jzIdx = code.size();
        code.push_back({(uint32_t)OpCode::JZ, (uint32_t)condReg, 0, 0});

        // body
        compileStatement(forStmt -> getBody(), code);

        // update
        compileStatement(forStmt -> getUpdate(), code);

        Instruction jmpFor = {(uint32_t)OpCode::JMP, 0, 0, 0};
        setAddress(jmpFor, (uint16_t)startAddr);
        code.push_back(jmpFor);
        setAddress(code[jzIdx], (uint16_t)code.size());
    }

    else if(auto funcDef = std::dynamic_pointer_cast<FunctionDefNode>(stmt)) {
        size_t jmpIdx = code.size();
        code.push_back({(uint32_t)OpCode::JMP, 0, 0, 0});

        size_t funcAddr = code.size();
        functionTable[funcDef->getName()] = {funcAddr, (int)funcDef->getParams().size()};

        int slots = funcDef->getLocalSlotCount();
        if (slots < 1) slots = 1;
        int frameSize = (slots + 4) * 4;

        // SP = SP - frameSize
        code.push_back({(uint32_t)OpCode::ADDI, SP, SP, (uint32_t)(int32_t)(-frameSize)});

        // FP = SP + frameSize
        code.push_back({(uint32_t)OpCode::ADDI, FP, SP, (uint32_t)frameSize});

        // պարամետրեր
        for(int i = 0; i < (int)funcDef->getParams().size(); i++) {
            std::string p = funcDef->getParams()[i];
            int32_t off = symTable.getLocalOffset(p);
            int reg = allocateTempRegister();

            code.push_back({(uint32_t)OpCode::LOAD_PARAM, (uint32_t)reg, (uint32_t)i, 0});
            code.push_back({(uint32_t)OpCode::STORE, (uint32_t)reg, (uint32_t)FP, (uint32_t)off});
        }

        compileStatement(funcDef->getBody(), code);

        // Epilogue
        code.push_back({(uint32_t)OpCode::ADDI, SP, SP, (uint32_t)frameSize});
        code.push_back({(uint32_t)OpCode::RETURN, 0, 0, 0});

        setAddress(code[jmpIdx], (uint16_t)code.size());
    } 
    
    else if(auto callStmt = std::dynamic_pointer_cast<FunctionCallStatementNode>(stmt)) {
        auto call = callStmt->getCall();

        for(const auto& arg : call->getArgs()) {
            globalCtx.consts.clear();
            globalCtx.vars.clear();
            auto exprCode = generateByteCode(postOrderTraverse(arg));
            code.insert(code.end(), exprCode.begin(), exprCode.end());
            int argReg = exprCode.empty() ? 0 : exprCode.back().dst;
            code.push_back({(uint32_t)OpCode::PUSH_ARG, (uint32_t)argReg, 0, 0});
        }

        int resultReg = allocateTempRegister();
        Instruction callInst;
        callInst.op  = (uint32_t)OpCode::CALL;
        callInst.dst = (uint32_t)resultReg;
        if(functionTable.count(call->getName())) {
            setAddress(callInst, (uint16_t)functionTable[call->getName()].address);
        } else {
            setAddress(callInst, 0);
            forwardCalls.push_back({code.size(), call->getName()});
        }
        code.push_back(callInst);
    }
     else if(auto retStmt = std::dynamic_pointer_cast<ReturnNode>(stmt)) {
        if(retStmt -> getExpression()) {
            auto exprCode = generateByteCode(postOrderTraverse(retStmt -> getExpression()));
            code.insert(code.end(), exprCode.begin(), exprCode.end());
            int lastReg = exprCode.empty() ? 0 : exprCode.back().dst;
            code.push_back({(uint32_t)OpCode::RETURN, (uint32_t)lastReg, 0, 0 });
        } else {
            code.push_back({(uint32_t)OpCode::RETURN, 0, 0, 0});
        }
    } 
}

void Compiler::printByteCode(const std::vector<Instruction>& code) const {
    for(const auto& inst : code)
        std::cout << "Op: " << (int)inst.op << " | L: " << inst.left
                  << " | R: " << inst.right << " | Dst: " << inst.dst << std::endl;
}

void writeByteCodeToFile(const ByteCode& bc, const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if(!out.is_open()) {
        throw std::runtime_error("Cannot open output file: " + path);
    }

    const char magic[4] = {'V', 'H', 'B', '1'};
    out.write(magic, sizeof(magic));

    uint32_t instructionCount = static_cast<uint32_t>(bc.instructions.size());
    uint32_t constantCount = static_cast<uint32_t>(bc.constants.size());
    uint32_t stringCount = static_cast<uint32_t>(bc.strings.size());

    out.write(reinterpret_cast<const char*>(&instructionCount), sizeof(instructionCount));
    out.write(reinterpret_cast<const char*>(&constantCount), sizeof(constantCount));
    out.write(reinterpret_cast<const char*>(&stringCount), sizeof(stringCount));

    for(const auto& inst : bc.instructions) {
        uint8_t op = static_cast<uint8_t>(inst.op);
        uint8_t dst = static_cast<uint8_t>(inst.dst);
        uint8_t left = static_cast<uint8_t>(inst.left);
        uint8_t right = static_cast<uint8_t>(inst.right);
        out.write(reinterpret_cast<const char*>(&op), sizeof(op));
        out.write(reinterpret_cast<const char*>(&dst), sizeof(dst));
        out.write(reinterpret_cast<const char*>(&left), sizeof(left));
        out.write(reinterpret_cast<const char*>(&right), sizeof(right));
    }

    for(double value : bc.constants) {
        out.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    for(const auto& str : bc.strings) {
        uint32_t len = static_cast<uint32_t>(str.size());
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        out.write(str.data(), len);
    }

    if(!out.good()) {
        throw std::runtime_error("Failed writing bytecode file: " + path);
    }
}

ByteCode readByteCodeFromFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if(!in.is_open()) {
        throw std::runtime_error("Cannot open bytecode file: " + path);
    }

    char magic[4] = {};
    in.read(magic, sizeof(magic));
    const char expected[4] = {'V', 'H', 'B', '1'};
    if(std::memcmp(magic, expected, sizeof(expected)) != 0) {
        throw std::runtime_error("Invalid bytecode format: " + path);
    }

    uint32_t instructionCount = 0;
    uint32_t constantCount = 0;
    uint32_t stringCount = 0;
    in.read(reinterpret_cast<char*>(&instructionCount), sizeof(instructionCount));
    in.read(reinterpret_cast<char*>(&constantCount), sizeof(constantCount));
    in.read(reinterpret_cast<char*>(&stringCount), sizeof(stringCount));

    ByteCode bc;
    bc.instructions.reserve(instructionCount);
    bc.constants.resize(constantCount);
    bc.strings.reserve(stringCount);

    for(uint32_t i = 0; i < instructionCount; ++i) {
        uint8_t op = 0, dst = 0, left = 0, right = 0;
        in.read(reinterpret_cast<char*>(&op), sizeof(op));
        in.read(reinterpret_cast<char*>(&dst), sizeof(dst));
        in.read(reinterpret_cast<char*>(&left), sizeof(left));
        in.read(reinterpret_cast<char*>(&right), sizeof(right));
        bc.instructions.push_back({op, dst, left, right});
    }

    for(uint32_t i = 0; i < constantCount; ++i) {
        in.read(reinterpret_cast<char*>(&bc.constants[i]), sizeof(double));
    }

    for(uint32_t i = 0; i < stringCount; ++i) {
        uint32_t len = 0;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        std::string s(len, '\0');
        if(len > 0) {
            in.read(&s[0], len);
        }
        bc.strings.push_back(std::move(s));
    }

    if(!in.good() && !in.eof()) {
        throw std::runtime_error("Failed reading bytecode file: " + path);
    }
    return bc;
}