#include "compiler.h"
#include <cmath>

// RISC-V register numbers
enum Reg : uint8_t {
    ZERO = 0, RA = 1, SP = 2, GP = 3, TP = 4,
    T0 = 5, T1 = 6, T2 = 7,
    FP = 8, S1 = 9,
    A0 = 10, A1 = 11, A2 = 12, A3 = 13, A4 = 14, A5 = 15, A6 = 16, A7 = 17,
    S2 = 18, S3 = 19, S4 = 20, S5 = 21, S6 = 22, S7 = 23, S8 = 24, S9 = 25, S10 = 26, S11 = 27,
    T3 = 28, T4 = 29, T5 = 30, T6 = 31
};

std::vector<std::shared_ptr<ASTNode>> Compiler::postOrderTraverse(std::shared_ptr<ASTNode> root) {
    if (!root) return {};
    std::vector<std::shared_ptr<ASTNode>> postOrder;
    std::stack<std::shared_ptr<ASTNode>> s1, s2;
    s1.push(root);
    while (!s1.empty()) {
        auto node = s1.top(); s1.pop(); s2.push(node);
        for (auto& child : node->getChildren()) s1.push(child);
    }
    while (!s2.empty()) { postOrder.push_back(s2.top()); s2.pop(); }
    return postOrder;
}

std::shared_ptr<ASTNode> Compiler::optimize(std::shared_ptr<ASTNode> node) {
    if (!node) return nullptr;

    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        auto left = optimize(bin->getLeft());
        auto right = optimize(bin->getRight());
        
        auto leftNum = std::dynamic_pointer_cast<NumberNode>(left);
        auto rightNum = std::dynamic_pointer_cast<NumberNode>(right);
        
        // If both children are numbers, compute constant result
        if (leftNum && rightNum) {
            double l = leftNum->getValue();
            double r = rightNum->getValue();
            double result = 0.0;
            std::string op = bin->getOp();
            
            // Arithmetic
            if (op == "+") result = l + r;
            else if (op == "-") result = l - r;
            else if (op == "*") result = l * r;
            else if (op == "/") {
                if (r == 0.0) result = 0.0; // division by zero -> 0 (or could throw, but fold to 0)
                else result = l / r;
            }
            else if (op == "%") {
                long long li = (long long)l, ri = (long long)r;
                result = (double)(li % ri);
            }
            // Bitwise
            else if (op == "&") result = (double)((long long)l & (long long)r);
            else if (op == "|") result = (double)((long long)l | (long long)r);
            else if (op == "^") result = (double)((long long)l ^ (long long)r);
            else if (op == "<<") result = (double)((long long)l << (long long)r);
            else if (op == ">>") result = (double)((long long)l >> (long long)r);
            // Comparisons
            else if (op == "==") result = (l == r) ? 1.0 : 0.0;
            else if (op == "!=") result = (l != r) ? 1.0 : 0.0;
            else if (op == "<")  result = (l < r)  ? 1.0 : 0.0;
            else if (op == ">")  result = (l > r)  ? 1.0 : 0.0;
            else if (op == "<=") result = (l <= r) ? 1.0 : 0.0;
            else if (op == ">=") result = (l >= r) ? 1.0 : 0.0;
            // Logical
            else if (op == "and") result = (l != 0.0 && r != 0.0) ? 1.0 : 0.0;
            else if (op == "or")  result = (l != 0.0 || r != 0.0) ? 1.0 : 0.0;
            // Power
            else if (op == "**") result = std::pow(l, r);
            else {
                // unknown operator, return original binary node with optimized children
                return std::make_shared<BinaryOpNode>(op, left, right);
            }
            return std::make_shared<NumberNode>(result);
        }
        // If not both numbers, return binary node with optimized children
        return std::make_shared<BinaryOpNode>(bin->getOp(), left, right);
    }
    
    if (auto un = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
        auto child = optimize(un->getChild());
        auto num = std::dynamic_pointer_cast<NumberNode>(child);
        if (num) {
            double val = num->getValue();
            std::string op = un->getOp();
            if (op == "-" || op == "_") {
                return std::make_shared<NumberNode>(-val);
            } else if (op == "+" || op == "#") {
                return child; // unary plus does nothing
            } else if (op == "not") {
                return std::make_shared<NumberNode>( (val == 0.0) ? 1.0 : 0.0 );
            }
        }
        return std::make_shared<UnaryOpNode>(un->getOp(), child);
    }
    
    if (auto assign = std::dynamic_pointer_cast<AssignmentNode>(node)) {
        auto optExpr = optimize(assign->getExpression());
        return std::make_shared<AssignmentNode>(assign->getOffset(), optExpr);
    }
    
    if (auto ifStmt = std::dynamic_pointer_cast<IfStatementNode>(node)) {
        auto cond = optimize(ifStmt->getCondition());
        auto thenBr = std::dynamic_pointer_cast<StatementNode>(optimize(ifStmt->getThenBr()));
        auto elseBr = std::dynamic_pointer_cast<StatementNode>(optimize(ifStmt->getElseBr()));
        // If condition is constant, eliminate dead branch
        if (auto condNum = std::dynamic_pointer_cast<NumberNode>(cond)) {
            if (condNum->getValue() != 0.0) {
                return thenBr ? thenBr : nullptr;
            } else {
                return elseBr ? elseBr : nullptr;
            }
        }
        return std::make_shared<IfStatementNode>(cond, thenBr, elseBr);
    }
    
    if (auto whileStmt = std::dynamic_pointer_cast<WhileStatementNode>(node)) {
        auto cond = optimize(whileStmt->getCondition());
        auto body = std::dynamic_pointer_cast<StatementNode>(optimize(whileStmt->getBody()));
        // If condition is constant false, eliminate whole loop
        if (auto condNum = std::dynamic_pointer_cast<NumberNode>(cond)) {
            if (condNum->getValue() == 0.0) {
                return nullptr; // loop never executes
            }
            // If condition is constant true, we could leave it, but we can't unroll
        }
        return std::make_shared<WhileStatementNode>(cond, body);
    }
    
    if (auto block = std::dynamic_pointer_cast<BlockCode>(node)) {
        auto newBlock = std::make_shared<BlockCode>();
        for (auto& stmt : block->getStatements()) {
            auto optStmt = std::dynamic_pointer_cast<StatementNode>(optimize(stmt));
            if (optStmt) newBlock->addStatement(optStmt);
        }
        return newBlock;
    }
    
    if (auto print = std::dynamic_pointer_cast<PrintNode>(node)) {
        std::vector<std::shared_ptr<ASTNode>> newExprs;
        for (auto& expr : print->getExpressions()) {
            newExprs.push_back(optimize(expr));
        }
        return std::make_shared<PrintNode>(newExprs);
    }
    
    if (auto forStmt = std::dynamic_pointer_cast<ForStatementNode>(node)) {
        auto init = std::dynamic_pointer_cast<StatementNode>(optimize(forStmt->getInit()));
        auto cond = optimize(forStmt->getCondition());
        auto update = std::dynamic_pointer_cast<StatementNode>(optimize(forStmt->getUpdate()));
        auto body = std::dynamic_pointer_cast<StatementNode>(optimize(forStmt->getBody()));
        return std::make_shared<ForStatementNode>(init, cond, update, body);
    }
    
    if (auto call = std::dynamic_pointer_cast<FunctionCallNode>(node)) {
        std::vector<std::shared_ptr<ASTNode>> newArgs;
        for (auto& arg : call->getArgs()) {
            newArgs.push_back(optimize(arg));
        }
        return std::make_shared<FunctionCallNode>(call->getName(), newArgs);
    }
    
    if (auto ret = std::dynamic_pointer_cast<ReturnNode>(node)) {
        auto optExpr = optimize(ret->getExpression());
        return std::make_shared<ReturnNode>(optExpr);
    }
    
    return node;
}

static OpCode getArithOp(const std::string& op) {
    if (op == "+") return OpCode::ADD;
    if (op == "-") return OpCode::SUB;
    if (op == "*") return OpCode::MUL;
    if (op == "/") return OpCode::DIV;
    if (op == "&") return OpCode::AND;
    if (op == "|") return OpCode::OR;
    if (op == "^") return OpCode::XOR;
    if (op == "%") return OpCode::MOD;
    if (op == "<<") return OpCode::SLL;
    if (op == ">>") return OpCode::SRL;
    if (op == "**") return OpCode::POW;
    return OpCode::UNDEFINED;
}

static OpCode getCompareOp(const std::string& op) {
    if (op == "==") return OpCode::CMP_EQ;
    if (op == "!=") return OpCode::CMP_NE;
    if (op == "<")  return OpCode::CMP_LT;
    if (op == ">")  return OpCode::CMP_GT;
    if (op == "<=") return OpCode::CMP_LE;
    if (op == ">=") return OpCode::CMP_GE;
    return OpCode::UNDEFINED;
}

std::vector<Instruction> Compiler::generateByteCode(const std::vector<std::shared_ptr<ASTNode>>& nodes, CompileContext& ctx) {
    std::vector<Instruction> code;
    std::stack<int> regStack;

    for (const auto& node : nodes) {
        if (auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
            double val = num->getValue();
            if (ctx.constMap.find(val) == ctx.constMap.end()) {
                int reg = ctx.nextReg++;
                int idx = (int)constantPool.size();
                constantPool.push_back(val);
                Instruction inst;
                inst.op = (uint32_t)OpCode::LOAD_CONST;
                inst.rd = (uint32_t)reg;
                inst.rs1 = (uint32_t)idx;
                inst.rs2 = 0;
                code.push_back(inst);
                ctx.constMap[val] = reg;
            }
            regStack.push(ctx.constMap[val]);
        }
        else if (auto var = std::dynamic_pointer_cast<VariableNode>(node)) {
            size_t off = var->getOffset();
            int rd = ctx.nextReg++;
            code.push_back({(uint32_t)OpCode::LW, (uint32_t)rd, (uint32_t)Reg::FP, (uint32_t)off});
            regStack.push(rd);
        }
        else if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
            int rightReg = regStack.top(); regStack.pop();
            int leftReg  = regStack.top(); regStack.pop();
            int destReg = ctx.nextReg++;
            std::string op = bin->getOp();
            if (op == "**") {
                code.push_back({(uint32_t)OpCode::POW, (uint32_t)destReg, (uint32_t)leftReg, (uint32_t)rightReg});
            }
            else if (op == "and" || op == "or") {
                OpCode logic = (op == "and") ? OpCode::LOGICAL_AND : OpCode::LOGICAL_OR;
                code.push_back({(uint32_t)logic, (uint32_t)destReg, (uint32_t)leftReg, (uint32_t)rightReg});
            }
            else if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
                OpCode cmp = getCompareOp(op);
                code.push_back({(uint32_t)cmp, (uint32_t)destReg, (uint32_t)leftReg, (uint32_t)rightReg});
            }
            else {
                OpCode arith = getArithOp(op);
                code.push_back({(uint32_t)arith, (uint32_t)destReg, (uint32_t)leftReg, (uint32_t)rightReg});
            }
            regStack.push(destReg);
        }
        else if (auto un = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
            int src = regStack.top(); regStack.pop();
            int dest = ctx.nextReg++;
            if (un->getOp() == "not") {
                code.push_back({(uint32_t)OpCode::LOGICAL_NOT, (uint32_t)dest, (uint32_t)src, 0});
            } else if (un->getOp() == "-") {
                code.push_back({(uint32_t)OpCode::SUB, (uint32_t)dest, (uint32_t)Reg::ZERO, (uint32_t)src});
            } else {
                code.push_back({(uint32_t)OpCode::ADD, (uint32_t)dest, (uint32_t)src, (uint32_t)Reg::ZERO});
            }
            regStack.push(dest);
        }
        else if (auto str = std::dynamic_pointer_cast<StringNode>(node)) {
            int idx = (int)stringPool.size();
            stringPool.push_back(str->getValue());
            int reg = ctx.nextReg++;
            code.push_back({(uint32_t)OpCode::LOAD_STR, (uint32_t)reg, (uint32_t)idx, 0});
            regStack.push(reg);
        }
        else if (auto call = std::dynamic_pointer_cast<FunctionCallNode>(node)) {
            auto& args = call->getArgs();

            // Evaluate arguments and store them in TEMPORARY registers (x5, x6, ...)
            std::vector<int> argRegs;
            for (size_t i = 0; i < args.size() && i < 8; ++i) {
                CompileContext argCtx;
                argCtx.nextReg = ctx.nextReg;
                auto argCode = generateByteCode(postOrderTraverse(args[i]), argCtx);
                code.insert(code.end(), argCode.begin(), argCode.end());
                int argReg = argCode.empty() ? 0 : argCode.back().rd;
                argRegs.push_back(argReg);
                ctx.nextReg = argCtx.nextReg;
            }

            // Move arguments to a0, a1, ... from temporary registers
            for (size_t i = 0; i < argRegs.size(); ++i) {
                int aReg = Reg::A0 + (int)i;
                code.push_back({(uint32_t)OpCode::ADD, (uint32_t)aReg, (uint32_t)argRegs[i], (uint32_t)Reg::ZERO});
            }

            // Function call: jal ra, funcAddr
            Instruction jal;
            jal.op = (uint32_t)OpCode::JAL;
            jal.rd = Reg::RA;
            if (functionTable.count(call->getName())) {
                setImmediate(jal, (uint16_t)functionTable[call->getName()].address);
            } else {
                setImmediate(jal, 0);
                forwardCalls.push_back({code.size(), call->getName()});
            }
            code.push_back(jal);

            // Result is in a0, move to a new temporary register
            int resultReg = ctx.nextReg++;
            code.push_back({(uint32_t)OpCode::ADD, (uint32_t)resultReg, (uint32_t)Reg::A0, (uint32_t)Reg::ZERO});
            regStack.push(resultReg);
        }
    }
    return code;
}

void Compiler::compileStatement(std::shared_ptr<StatementNode> stmt, std::vector<Instruction>& code, CompileContext& ctx) {
    if (!stmt) return;

    if (auto assign = std::dynamic_pointer_cast<AssignmentNode>(stmt)) {
        CompileContext exprCtx;
        auto exprCode = generateByteCode(postOrderTraverse(assign->getExpression()), exprCtx);
        code.insert(code.end(), exprCode.begin(), exprCode.end());
        int valReg = exprCode.empty() ? 0 : exprCode.back().rd;
        code.push_back({(uint32_t)OpCode::SW, (uint32_t)valReg, (uint32_t)Reg::FP, (uint32_t)assign->getOffset()});
    }
    else if (auto ifStmt = std::dynamic_pointer_cast<IfStatementNode>(stmt)) {
    CompileContext condCtx;
    auto condCode = generateByteCode(postOrderTraverse(ifStmt->getCondition()), condCtx);
    code.insert(code.end(), condCode.begin(), condCode.end());
    int condReg = condCode.empty() ? 0 : condCode.back().rd;
    
    // BEQ condReg, zero, elseLabel
    size_t beqIdx = code.size();
    Instruction beq;
    beq.op = (uint32_t)OpCode::BEQ;
    beq.rd = (uint32_t)condReg;
    beq.rs1 = (uint32_t)Reg::ZERO;
    setImmediate(beq, 0); // placeholder
    code.push_back(beq);
    
    compileStatement(ifStmt->getThenBr(), code, ctx);
    
    // JMP endLabel
    size_t jmpIdx = code.size();
    Instruction jmp;
    jmp.op = (uint32_t)OpCode::JAL;
    jmp.rd = (uint32_t)Reg::ZERO;
    setImmediate(jmp, 0); // placeholder
    code.push_back(jmp);
    
    // Fix BEQ target to current position (else block starts here)
    setImmediate(code[beqIdx], (uint16_t)code.size());
    
    if (ifStmt->getElseBr()) {
        compileStatement(ifStmt->getElseBr(), code, ctx);
    }
    // Fix JMP target to current position (after else block)
    setImmediate(code[jmpIdx], (uint16_t)code.size());
}
    else if (auto whileStmt = std::dynamic_pointer_cast<WhileStatementNode>(stmt)) {
    size_t startAddr = code.size();
    CompileContext condCtx;
    auto condCode = generateByteCode(postOrderTraverse(whileStmt->getCondition()), condCtx);
    code.insert(code.end(), condCode.begin(), condCode.end());
    int condReg = condCode.empty() ? 0 : condCode.back().rd;
       
    // BEQ condReg, zero, exitLabel
    size_t beqIdx = code.size();
    Instruction beq;
    beq.op = (uint32_t)OpCode::BEQ;
    beq.rd = (uint32_t)condReg;
    beq.rs1 = (uint32_t)Reg::ZERO;
    setImmediate(beq, 0);
    code.push_back(beq);
    
    compileStatement(whileStmt->getBody(), code, ctx);
    
    // JMP startAddr
    Instruction jmp;
    jmp.op = (uint32_t)OpCode::JAL;
    jmp.rd = (uint32_t)Reg::ZERO;
    setImmediate(jmp, (uint16_t)startAddr);
    code.push_back(jmp);
    
    // Fix BEQ target to exit label (after the loop)
    setImmediate(code[beqIdx], (uint16_t)code.size());
}
    else if (auto block = std::dynamic_pointer_cast<BlockCode>(stmt)) {
        symTable->enterScope();
        for (auto& s : block->getStatements()) {
            compileStatement(s, code, ctx);
        }
        symTable->exitScope();
    }
    else if (auto printStmt = std::dynamic_pointer_cast<PrintNode>(stmt)) {
        for (const auto& expr : printStmt->getExpressions()) {
            if (auto strNode = std::dynamic_pointer_cast<StringNode>(expr)) {
                int idx = (int)stringPool.size();
                stringPool.push_back(strNode->getValue());
                code.push_back({(uint32_t)OpCode::PRINT_STR, (uint32_t)idx, 0, 0});
            } else {
                CompileContext exprCtx;
                auto exprCode = generateByteCode(postOrderTraverse(expr), exprCtx);
                code.insert(code.end(), exprCode.begin(), exprCode.end());
                int valReg = exprCode.empty() ? 0 : exprCode.back().rd;
                code.push_back({(uint32_t)OpCode::PRINT, (uint32_t)valReg, 0, 0});
            }
        }
    }
    else if (auto forStmt = std::dynamic_pointer_cast<ForStatementNode>(stmt)) {
        // Initialize: i = start
        compileStatement(forStmt->getInit(), code, ctx);
        
        // Loop start address
        size_t startAddr = code.size();
        
        // Condition
        CompileContext condCtx;
        auto condCode = generateByteCode(postOrderTraverse(forStmt->getCondition()), condCtx);
        code.insert(code.end(), condCode.begin(), condCode.end());
        int condReg = condCode.empty() ? 0 : condCode.back().rd;
        
        // BEQ condReg, zero, exitLabel (jump out if condition is false)
        size_t beqIdx = code.size();
        Instruction beq;
        beq.op = (uint32_t)OpCode::BEQ;
        beq.rd = (uint32_t)condReg;
        beq.rs1 = (uint32_t)Reg::ZERO;
        setImmediate(beq, 0);  // placeholder, will be fixed later
        code.push_back(beq);
        
        // Body
        compileStatement(forStmt->getBody(), code, ctx);
        
        // Update: i = i + 1 (or similar)
        compileStatement(forStmt->getUpdate(), code, ctx);
        
        // Jump back to condition
        Instruction jmp;
        jmp.op = (uint32_t)OpCode::JAL;
        jmp.rd = (uint32_t)Reg::ZERO;
        setImmediate(jmp, (uint16_t)startAddr);
        code.push_back(jmp);
        
        // Fix BEQ target to current position (exit label - after the loop)
        setImmediate(code[beqIdx], (uint16_t)code.size());
    }
    else if (auto funcDef = std::dynamic_pointer_cast<FunctionDefNode>(stmt)) {
        size_t jmpIdx = code.size();
        code.push_back({(uint32_t)OpCode::JAL, (uint32_t)Reg::ZERO, 0, 0}); // skip over body

        size_t funcAddr = code.size();
        int paramCount = (int)funcDef->getParams().size();

        symTable->enterScope();
        CompileContext funcCtx;

        int frameSize = (int)symTable->getCurrentScopeSize(); // includes params
        // prologue: addi sp, sp, -frameSize*4
        code.push_back({(uint32_t)OpCode::ADDI, (uint32_t)Reg::SP, (uint32_t)Reg::SP, (uint32_t)(-frameSize * 4)});
        // sw ra, (frameSize-1)*4(sp)
        code.push_back({(uint32_t)OpCode::SW, (uint32_t)Reg::RA, (uint32_t)Reg::SP, (uint32_t)((frameSize-1)*4)});
        // sw fp, (frameSize-2)*4(sp)
        code.push_back({(uint32_t)OpCode::SW, (uint32_t)Reg::FP, (uint32_t)Reg::SP, (uint32_t)((frameSize-2)*4)});
        // addi fp, sp, frameSize*4
        code.push_back({(uint32_t)OpCode::ADDI, (uint32_t)Reg::FP, (uint32_t)Reg::SP, (uint32_t)(frameSize * 4)});

        // store parameters into local variables (offsets)
        for (int i = 0; i < paramCount; ++i) {
            size_t off = symTable->getOffset(funcDef->getParams()[i]);
            int argReg = Reg::A0 + i;
            code.push_back({(uint32_t)OpCode::SW, (uint32_t)argReg, (uint32_t)Reg::FP, (uint32_t)off});
        }

        compileStatement(funcDef->getBody(), code, funcCtx);

        // epilogue
        // lw ra, (frameSize-1)*4(sp)
        code.push_back({(uint32_t)OpCode::LW, (uint32_t)Reg::RA, (uint32_t)Reg::SP, (uint32_t)((frameSize-1)*4)});
        // lw fp, (frameSize-2)*4(sp)
        code.push_back({(uint32_t)OpCode::LW, (uint32_t)Reg::FP, (uint32_t)Reg::SP, (uint32_t)((frameSize-2)*4)});
        // addi sp, sp, frameSize*4
        code.push_back({(uint32_t)OpCode::ADDI, (uint32_t)Reg::SP, (uint32_t)Reg::SP, (uint32_t)(frameSize * 4)});
        // ret: jalr x0, ra, 0
        code.push_back({(uint32_t)OpCode::JALR, (uint32_t)Reg::ZERO, (uint32_t)Reg::RA, 0});

        functionTable[funcDef->getName()] = {funcAddr, paramCount, frameSize};
        symTable->exitScope();

        setImmediate(code[jmpIdx], (uint16_t)code.size());
    }
    else if (auto callStmt = std::dynamic_pointer_cast<FunctionCallStatementNode>(stmt)) {
        auto call = callStmt->getCall();
        for (size_t i = 0; i < call->getArgs().size() && i < 8; ++i) {
            CompileContext argCtx;
            argCtx.nextReg = ctx.nextReg;
            auto argCode = generateByteCode(postOrderTraverse(call->getArgs()[i]), argCtx);
            code.insert(code.end(), argCode.begin(), argCode.end());
            int argReg = argCode.empty() ? 0 : argCode.back().rd;
            int aReg = Reg::A0 + (int)i;
            code.push_back({(uint32_t)OpCode::ADD, (uint32_t)aReg, (uint32_t)argReg, (uint32_t)Reg::ZERO});
            ctx.nextReg = argCtx.nextReg;
        }
        Instruction jal;
        jal.op = (uint32_t)OpCode::JAL;
        jal.rd = Reg::RA;
        if (functionTable.count(call->getName())) {
            setImmediate(jal, (uint16_t)functionTable[call->getName()].address);
        } else {
            setImmediate(jal, 0);
            forwardCalls.push_back({code.size(), call->getName()});
        }
        code.push_back(jal);
        // result ignored
    }
    else if (auto retStmt = std::dynamic_pointer_cast<ReturnNode>(stmt)) {
        if (retStmt->getExpression()) {
            CompileContext retCtx;
            auto exprCode = generateByteCode(postOrderTraverse(retStmt->getExpression()), retCtx);
            code.insert(code.end(), exprCode.begin(), exprCode.end());
            int valReg = exprCode.empty() ? 0 : exprCode.back().rd;
            code.push_back({(uint32_t)OpCode::ADD, (uint32_t)Reg::A0, (uint32_t)valReg, (uint32_t)Reg::ZERO});
        }
        // After return we should jump to epilogue; but epilogue will be executed anyway
        // To avoid executing code after return, we can add an unconditional jump to epilogue.
        // For simplicity, assume return is last statement of function.
    }
}

ByteCode Compiler::compile(std::shared_ptr<ASTNode> root) {
    constantPool.clear();
    stringPool.clear();
    functionTable.clear();
    forwardCalls.clear();
    nextLabel = 0;

    std::vector<Instruction> insts;
    if (!root) return {insts, constantPool, stringPool};

    auto optimizedRoot = optimize(root);
    auto stmt = std::dynamic_pointer_cast<StatementNode>(optimizedRoot);
    CompileContext ctx;
    if (stmt) compileStatement(stmt, insts, ctx);

    for (auto& [idx, name] : forwardCalls) {
        if (functionTable.count(name)) {
            setImmediate(insts[idx], (uint16_t)functionTable[name].address);
        } else {
            throw std::runtime_error("Undefined function: " + name);
        }
    }
    insts.push_back({(uint32_t)OpCode::HALT, 0, 0, 0});
    return {insts, constantPool, stringPool};
}

void Compiler::printByteCode(const std::vector<Instruction>& code) const {
    for (size_t i = 0; i < code.size(); ++i) {
        std::cout << i << ": op=" << (int)code[i].op
                  << " rd=" << (int)code[i].rd
                  << " rs1=" << (int)code[i].rs1
                  << " rs2=" << (int)code[i].rs2 << std::endl;
    }
}