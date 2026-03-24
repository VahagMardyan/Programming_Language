#include "compiler.h"

std::vector<std::shared_ptr<ASTNode>> Compiler::postOrderTraverse(std::shared_ptr<ASTNode> root) {
    if(!root) return {};
    std::vector<std::shared_ptr<ASTNode>> postOrder;
    std::stack<std::shared_ptr<ASTNode>> s1, s2;

    s1.push(root);
    while(!s1.empty()) {
        auto node = s1.top();
        s1.pop();
        s2.push(node);
        for(auto& child : node -> getChildren()) {
            s1.push(child);
        }
    }

    while(!s2.empty()) {
        postOrder.push_back(s2.top());
        s2.pop();
    }

    return postOrder;
}

std::shared_ptr<ASTNode> Compiler::optimize(std::shared_ptr<ASTNode> node) {
    if(!node) {
        return nullptr;
    }
    if(auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        auto left = optimize(bin -> getLeft());
        auto right = optimize(bin -> getRight());

        auto leftNum = std::dynamic_pointer_cast<NumberNode>(left);
        auto rightNum = std::dynamic_pointer_cast<NumberNode>(right);

        if(leftNum && rightNum) {
            double v1 = leftNum -> getValue();
            double v2 = rightNum -> getValue();
            double result = 0;
            switch(bin -> getOpCode()) {
                case OpCode::ADD: result = v1 + v2; break;
                case OpCode::SUB: result = v1 - v2; break;
                case OpCode::MUL: result = v1 * v2; break;
                case OpCode::DIV: result = (v2 == 0) ? 0 : v1 / v2; break;
                case OpCode::AND: 
                    result = static_cast<double>(static_cast<long long>(v1) & static_cast<long long>(v2));
                break;
                case OpCode::OR: 
                    result = static_cast<double>(static_cast<long long>(v1) | static_cast<long long>(v2));
                break;
                case OpCode::XOR: 
                    result = static_cast<double>(static_cast<long long>(v1) ^ static_cast<long long>(v2));
                break;
                case OpCode::MODULO: 
                    result = static_cast<double>(static_cast<long long>(v1) % static_cast<long long>(v2));
                break;
                case OpCode::LSHIFT: 
                    result = static_cast<double>(static_cast<long long>(v1) << static_cast<long long>(v2));
                break;
                case OpCode::RSHIFT: 
                    result = static_cast<double>(static_cast<long long>(v1) >> static_cast<long long>(v2));
                break;
                default: break;
            }
            return std::make_shared<NumberNode>(result);
        }

        if (auto leftBin = std::dynamic_pointer_cast<BinaryOpNode>(left)) {
            auto leftRightNum = std::dynamic_pointer_cast<NumberNode>(leftBin->getRight());
            if (leftRightNum && rightNum && bin->getOpCode() == OpCode::ADD && leftBin->getOpCode() == OpCode::ADD) {
                double sum = leftRightNum->getValue() + rightNum->getValue();
                return std::make_shared<BinaryOpNode>(bin->getOp(), leftBin->getLeft(), std::make_shared<NumberNode>(sum));
            }
        }
        return std::make_shared<BinaryOpNode>(bin -> getOp(), left, right);
    }
    return node;
}

ByteCode Compiler::compile(std::shared_ptr<ASTNode> root) {
    auto optimizedRoot = optimize(root);

    auto postOrder = postOrderTraverse(optimizedRoot);
    std::vector<Instruction> insts = generateByteCode(postOrder);
    return {
        insts, constantPool
    };
}

std::vector<Instruction> Compiler::generateByteCode(const std::vector<std::shared_ptr<ASTNode>>& nodes) {
    std::vector<Instruction> code;
    std::stack<int> storage;
    CompileContext ctx;
    nextTempIndex = 0;
    constantPool.clear();
    for(const auto& node: nodes) {
        if(auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
            double val = num -> getValue();
            if(ctx.consts.find(val) == ctx.consts.end()) {
                int reg = nextTempIndex++;
                int constIdx = (int)constantPool.size();
                constantPool.push_back(val);
                
                code.push_back({
                    (uint32_t)OpCode::LOAD_CONST, (uint32_t)reg, (uint32_t)constIdx, 0
                });
                ctx.consts[val] = reg;
            }
            storage.push(ctx.consts[val]);
        } else if(auto var = std::dynamic_pointer_cast<VariableNode>(node)) {
            size_t addr = var -> get_address();
            if(ctx.vars.find(addr) == ctx.vars.end()) {
                int reg = nextTempIndex++;
                code.push_back({
                    (uint32_t)OpCode::LOAD_VAR, (uint32_t)reg, (uint32_t)addr, 0
                });
                ctx.vars[addr] = reg;
            }
            storage.push(ctx.vars[addr]);
        } else if(auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
            int r = storage.top(); storage.pop();
            int l = storage.top(); storage.pop();
            int target = nextTempIndex++;
            code.push_back({
                (uint32_t)bin -> getOpCode(), (uint32_t)target, (uint32_t)l, (uint32_t)r
            });
            storage.push(target);
        } else if(auto un = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
            int childIdx = storage.top(); storage.pop();
            int target = nextTempIndex++;
            code.push_back({
                (uint32_t)OpCode::UNARY, (uint32_t)target, (uint32_t)childIdx, 0
            });
            storage.push(target);
        }
    }
    return code;
}

void Compiler::printByteCode(const std::vector<Instruction>& code) const {
    std::cout << "Generated ByteCode" << std::endl;

    for(const auto& inst : code) {
        std::cout << "Op: " << (int)inst.op 
                << " | L: " << inst.left 
                << " | R: " << inst.right 
                << " | Dest: " << inst.dst
                << " | Val: " << constantPool[inst.left] << std::endl;
    }
}