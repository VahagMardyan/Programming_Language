#include "parser.h"
#include <iostream>

Parser::Parser(Tokenizer& tok, SymbolTable& st) : tokenizer(tok), symTable(st), state(ParserState::ExpectOperand) {}

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
    std::string op = ops.top();
    ops.pop();
    if(op == "_" || op == "#") {
        auto operand = nodes.top();
        nodes.pop();
        nodes.push(std::make_shared<UnaryOpNode>(op, operand));
    } else {
        auto right = nodes.top();
        nodes.pop();
        auto left = nodes.top();
        nodes.pop();
        nodes.push(std::make_shared<BinaryOpNode>(op, left, right));
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
            if(state != ParserState::Error) state = ParserState::Done;
            break;
        }

        if(token.type == TokenType::Error) {
            state = ParserState::Error;
            break;
        }

        switch(state) {
            case ParserState::ExpectOperand:
                if(token.type == TokenType::Number) {
                    nodes.push(std::make_shared<NumberNode>(std::stod(token.value)));
                    state = ParserState::ExpectOperator;
                } else if(token.type == TokenType::Name) {
                    nodes.push(std::make_shared<VariableNode>(symTable.getAddress(token.value), symTable));
                    state = ParserState::ExpectOperator;
                } else if(token.type == TokenType::OpenParen) {
                    ops.push("(");
                } else if(token.type == TokenType::Operator && (token.value == "-" || token.value == "+")) {
                    ops.push(token.value == "-" ? "_" : "#");
                } else {
                    state = ParserState::Error;
                }
            break;

            case ParserState::ExpectOperator:
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
                } else {
                    state = ParserState::Error;
                }
            break;
        }
    }

    if (state == ParserState::Error) {
        return nullptr;
    }
    
    return nodes.empty() ? nullptr : nodes.top();
}