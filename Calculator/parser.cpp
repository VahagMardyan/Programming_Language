#include "parser.h"
#include <iostream>

std::shared_ptr<ASTNode> Parser::createBinaryNode(const std::string& op, std::shared_ptr<ASTNode> left, 
                std::shared_ptr<ASTNode> right) {

    auto leftNum = std::dynamic_pointer_cast<NumberNode>(left);
    auto rightNum = std::dynamic_pointer_cast<NumberNode>(right);
    if(leftNum && rightNum) {
        double lVal = leftNum -> getValue();
        double rVal = rightNum -> getValue();

        if(op == "+") return std::make_shared<NumberNode>(lVal + rVal);
        if (op == "-") return std::make_shared<NumberNode>(lVal - rVal);
        if (op == "*") return std::make_shared<NumberNode>(lVal * rVal);
        if (op == "/") return std::make_shared<NumberNode>(rVal != 0 ? lVal / rVal : 0);

        long long lInt = static_cast<long long>(lVal);
        long long rInt = static_cast<long long>(rVal);
        if (op == "&") return std::make_shared<NumberNode>(static_cast<double>(lInt & rInt));
        if (op == "|") return std::make_shared<NumberNode>(static_cast<double>(lInt | rInt));
        if (op == "^") return std::make_shared<NumberNode>(static_cast<double>(lInt ^ rInt));
        if (op == "<<") return std::make_shared<NumberNode>(static_cast<double>(lInt << rInt));
        if (op == ">>") return std::make_shared<NumberNode>(static_cast<double>(lInt >> rInt));
        if (op == "%") return std::make_shared<NumberNode>(static_cast<double>(lInt % rInt));
    }
    return std::make_shared<BinaryOpNode>(op, left, right);
}

std::shared_ptr<ASTNode> Parser::createUnaryNode(const std::string& op, std::shared_ptr<ASTNode>child) {
    auto num = std::dynamic_pointer_cast<NumberNode>(child);
    if(num) {
        double val = num -> getValue();
        if(op == "-" || op == "_") return std::make_shared<NumberNode>(-val);
        if(op == "+" || op == "#") return num;
    }
    return std::make_shared<UnaryOpNode>(op, child);
}

int Parser::precedence(const std::string& op) const {
    if(op == "|" || op == "^") return 1;
    if(op == "&") return 2;
    if(op == "<<" || op == ">>") return 3;
    if(op == "+" || op == "-") return 4;
    if(op == "*" || op == "/" || op == "%") return 5;
    if(op == "_" || op == "#") return 6; // unary operators
    return 0;
}

void Parser::createNodeFromOp() {
    if(ops.empty()) return;
    std::string op = ops.top();
    ops.pop();

    if(op == "_" || op == "#") {
        if(nodes.empty()) {
            state = ParserState::Error;
            return;
        }
        auto operand = nodes.top();
        nodes.pop();

        nodes.push(createUnaryNode(op, operand));
    } else {
        if(nodes.size() < 2) {
            state = ParserState::Error;
            return;
        }
        auto right = nodes.top();
        nodes.pop();
        auto left = nodes.top();
        nodes.pop();
        nodes.push(createBinaryNode(op, left, right));
    }
}

void Parser::processOperatorStack(const std::string& currentOp) {
    while(!ops.empty() && 
        ops.top() != "(" && precedence(ops.top()) >= precedence(currentOp)) {
        createNodeFromOp();
    }
}

std::shared_ptr<ASTNode> Parser::parse() {
    while(state != ParserState::Done && state != ParserState::Error) {
        Token token = tokenizer.getNextToken();
        if(token.type == TokenType::EndOfExpr) {
            while(!ops.empty()) {
                if(ops.top() == "(") {
                    state = ParserState::Error;
                    break;
                }
                createNodeFromOp();
            }
            if(state != ParserState::Error) {
                state = ParserState::Done;
            }
            break;
        }

        if(token.type == TokenType::Error) {
            state = ParserState::Error;
            break;
        }

        switch (state) {
            case ParserState::ExpectOperand: {
                if(token.type == TokenType::Number) {
                    nodes.push(std::make_shared<NumberNode>(std::stod(token.value)));
                    state = ParserState::ExpectOperator;
                } else if(token.type == TokenType::Name) {
                    nodes.push(std::make_shared<VariableNode>(symTable.getAddress(token.value)));
                    state = ParserState::ExpectOperator;
                } else if(token.type == TokenType::OpenParen) {
                    ops.push("(");
                } else if(token.type == TokenType::Operator && (token.value == "-" || token.value == "+")) {
                    ops.push(token.value == "-" ? "_" : "#");
                } else {
                    state = ParserState::Error;
                }
            }
            break;

            case ParserState::ExpectOperator: {
                if(token.type == TokenType::Operator) {
                    processOperatorStack(token.value);
                    ops.push(token.value);
                    state = ParserState::ExpectOperand;
                } else if(token.type == TokenType::CloseParen) {
                    while(!ops.empty() && ops.top() != "(") {
                        createNodeFromOp();
                    }
                    if(!ops.empty() && ops.top() == "(") {
                        ops.pop();
                        state = ParserState::ExpectOperator;
                    } else {
                        state = ParserState::Error;
                    }
                } else if(token.type == TokenType::Number || token.type == TokenType::Name || token.type == TokenType::OpenParen) {
                    processOperatorStack("*");
                    ops.push("*");
                    if(token.type == TokenType::Number) {
                        nodes.push(std::make_shared<NumberNode>(std::stod(token.value)));
                        state = ParserState::ExpectOperator;
                    } else if(token.type == TokenType::Name) {
                        nodes.push(std::make_shared<VariableNode>(symTable.getAddress(token.value)));
                        state = ParserState::ExpectOperator;
                    } else if(token.type == TokenType::OpenParen) {
                        ops.push("(");
                        state = ParserState::ExpectOperand;
                    } else {
                        state = ParserState::Error;
                    }
                }
            }
            break;
        }
    }
    if(state == ParserState::Error || nodes.empty()) {
        return nullptr;
    }
    return nodes.top();
}