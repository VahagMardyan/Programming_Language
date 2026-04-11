#include "compiler.h"

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
                case OpCode::DIV: result = v2 ? v1/v2 : 0; break;
                case OpCode::AND: result = (double)((long long)v1 & (long long)v2); break;
                case OpCode::OR:  result = (double)((long long)v1 | (long long)v2); break;
                case OpCode::XOR: result = (double)((long long)v1 ^ (long long)v2); break;
                case OpCode::MODULO: result = (double)((long long)v1 % (long long)v2); break;
                case OpCode::LSHIFT: result = (double)((long long)v1 << (long long)v2); break;
                case OpCode::RSHIFT: result = (double)((long long)v1 >> (long long)v2); break;
                case OpCode::LOGICAL_AND: result = (v1 != 0 && v2 != 0) ? 1.0 : 0.0; break;
                case OpCode::LOGICAL_OR: result = (v1 != 0 || v2 != 0) ? 1.0 : 0.0; break;
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
        return std::make_shared<AssignmentNode>(assign->getAddress(), optimize(assign->getExpression()));
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
    nextTempIndex = 0;
    std::vector<Instruction> insts;
    if(!root) return {insts, constantPool};
    auto optimizedRoot = optimize(root);
    auto stmt = std::dynamic_pointer_cast<StatementNode>(optimizedRoot);
    if(stmt) compileStatement(stmt, insts);
    else { auto po = postOrderTraverse(optimizedRoot); insts = generateByteCode(po); }
    return {insts ,constantPool, stringPool};
}

std::vector<Instruction> Compiler::generateByteCode(const std::vector<std::shared_ptr<ASTNode>>& nodes) {
    std::vector<Instruction> code;
    std::stack<int> storage;
    for(const auto& node : nodes) {
        if(auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
            double val = num->getValue();
            if(globalCtx.consts.find(val) == globalCtx.consts.end()) {
                int reg = nextTempIndex++, idx = (int)constantPool.size();
                constantPool.push_back(val);
                code.push_back({(uint32_t)OpCode::LOAD_CONST, (uint32_t)reg, (uint32_t)idx, 0});
                globalCtx.consts[val] = reg;
            }
            storage.push(globalCtx.consts[val]);
        } else if(auto var = std::dynamic_pointer_cast<VariableNode>(node)) {
            size_t addr = var->get_address();
            if(globalCtx.vars.find(addr) == globalCtx.vars.end()) {
                int reg = nextTempIndex++;
                code.push_back({(uint32_t)OpCode::LOAD_VAR, (uint32_t)reg, (uint32_t)addr, 0});
                globalCtx.vars[addr] = reg;
            }
            storage.push(globalCtx.vars[addr]);
        } else if(auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
            int r = storage.top(); storage.pop();
            int l = storage.top(); storage.pop();
            int target = nextTempIndex++;
            code.push_back({(uint32_t)bin->getOpCode(), (uint32_t)target, (uint32_t)l, (uint32_t)r});
            storage.push(target);
        } else if(auto un = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
            int childIdx = storage.top(); storage.pop();
            int target = nextTempIndex++;
            code.push_back({(uint32_t)OpCode::UNARY, (uint32_t)target, (uint32_t)childIdx, 0});
            storage.push(target);
        } else if(auto strNode = std::dynamic_pointer_cast<StringNode>(node)) {
            int strIdx = (int)stringPool.size();
            stringPool.push_back(strNode -> getValue());
            int reg = nextTempIndex++;
            code.push_back({
                (uint32_t)OpCode::LOAD_STR, (uint32_t)reg, (uint32_t)strIdx, 0
            });
            storage.push(reg);
        }
    }
    return code;
}

void Compiler::compileStatement(std::shared_ptr<StatementNode> stmt, std::vector<Instruction>& code) {
    if(!stmt) return;
    if(auto assign = std::dynamic_pointer_cast<AssignmentNode>(stmt)) {
        globalCtx.consts.clear(); globalCtx.vars.clear();
        auto exprCode = generateByteCode(postOrderTraverse(assign->getExpression()));
        code.insert(code.end(), exprCode.begin(), exprCode.end());
        int lastReg = exprCode.empty() ? 0 : exprCode.back().dst;
        code.push_back({(uint32_t)OpCode::STORE_VAR, 0, (uint32_t)assign->getAddress(), (uint32_t)lastReg});
    } else if(auto ifStmt = std::dynamic_pointer_cast<IfStatementNode>(stmt)) {
        globalCtx.consts.clear(); globalCtx.vars.clear();
        auto condCode = generateByteCode(postOrderTraverse(ifStmt->getCondition()));
        code.insert(code.end(), condCode.begin(), condCode.end());
        int condReg = condCode.empty() ? 0 : condCode.back().dst;
        size_t jzIdx = code.size();
        code.push_back({(uint32_t)OpCode::JZ, (uint32_t)condReg, 0, 0});
        compileStatement(ifStmt->getThenBr(), code);
        size_t jmpIdx = code.size();
        code.push_back({(uint32_t)OpCode::JMP, 0, 0, 0});
        code[jzIdx].left = (uint32_t)code.size();
        if(ifStmt->getElseBr()) compileStatement(ifStmt->getElseBr(), code);
        code[jmpIdx].left = (uint32_t)code.size();
    } else if(auto whileStmt = std::dynamic_pointer_cast<WhileStatementNode>(stmt)) {
        size_t startAddr = code.size();
        globalCtx.consts.clear(); globalCtx.vars.clear();
        auto condCode = generateByteCode(postOrderTraverse(whileStmt->getCondition()));
        code.insert(code.end(), condCode.begin(), condCode.end());
        int condReg = condCode.empty() ? 0 : condCode.back().dst;
        size_t jzIdx = code.size();
        code.push_back({(uint32_t)OpCode::JZ, (uint32_t)condReg, 0, 0});
        compileStatement(whileStmt->getBody(), code);
        code.push_back({(uint32_t)OpCode::JMP, 0, (uint32_t)startAddr, 0});
        code[jzIdx].left = (uint32_t)code.size();
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

        code.push_back({(uint32_t)OpCode::JMP, 0, (uint32_t)startAddr, 0});
        code[jzIdx].left = (uint32_t)code.size();
    }
}

void Compiler::printByteCode(const std::vector<Instruction>& code) const {
    for(const auto& inst : code)
        std::cout << "Op: " << (int)inst.op << " | L: " << inst.left
                  << " | R: " << inst.right << " | Dst: " << inst.dst << std::endl;
}