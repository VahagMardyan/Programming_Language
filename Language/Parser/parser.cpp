#include "parser.h"

std::shared_ptr<ASTNode> Parser::createBinaryNode(const std::string& op,
    std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right) {
    auto leftNum  = std::dynamic_pointer_cast<NumberNode>(left);
    auto rightNum = std::dynamic_pointer_cast<NumberNode>(right);
    if(leftNum && rightNum) {
        double l = leftNum->getValue(), r = rightNum->getValue();
        if(op == "+") return std::make_shared<NumberNode>(l + r);
        if(op == "-") return std::make_shared<NumberNode>(l - r);
        if(op == "*") return std::make_shared<NumberNode>(l * r);
        if(op == "**") return std::make_shared<NumberNode>(std::pow(l,r));
        if(op == "/") return std::make_shared<NumberNode>(r != 0 ? l / r : 0);
        long long li = (long long)l, ri = (long long)r;
        if(op == "&")  return std::make_shared<NumberNode>((double)(li & ri));
        if(op == "|")  return std::make_shared<NumberNode>((double)(li | ri));
        if(op == "^")  return std::make_shared<NumberNode>((double)(li ^ ri));
        if(op == "<<") return std::make_shared<NumberNode>((double)(li << ri));
        if(op == ">>") return std::make_shared<NumberNode>((double)(li >> ri));
        if(op == "%")  return std::make_shared<NumberNode>((double)(li % ri));
        if(op == "and") {
            return std::make_shared<NumberNode>((l != 0 && r != 0) ? 1.0 : 0.0);
        }
        if(op == "or") {
            return std::make_shared<NumberNode>((l != 0 || r != 0) ? 1.0 : 0.0);
        }
    }
    return std::make_shared<BinaryOpNode>(op, left, right);
}

std::shared_ptr<ASTNode> Parser::createUnaryNode(const std::string& op,
    std::shared_ptr<ASTNode> child) {
    auto num = std::dynamic_pointer_cast<NumberNode>(child);
    if(num) {
        if(op == "-" || op == "_") return std::make_shared<NumberNode>(-num->getValue());
        if(op == "+" || op == "#") return num;
    }
    if(op == "not") {
        if(num) return std::make_shared<NumberNode>(num -> getValue() == 0 ? 1.0 : 0.0);
        return std::make_shared<UnaryOpNode>("not", child);
    }
    return std::make_shared<UnaryOpNode>(op, child);
}

int Parser::precedence(const std::string& op) const {
    if(op == "or") return 0;
    if(op == "and") return 1;
    if(op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=") return 2;
    if(op == "|" || op == "^") return 3;
    if(op == "&") return 4;
    if(op == "<<" || op == ">>") return 5;
    if(op == "+" || op == "-") return 6;
    if(op == "*" || op == "/" || op == "%") return 7;
    if(op == "**") return 8;
    if(op == "not" || op == "_" || op == "#") return 9;
    return -1;
}

void Parser::createNodeFromOp() {
    if(ops.empty()) return;
    std::string op = ops.top(); ops.pop();
    if(op == "_" || op == "#" || op == "not") {
        if(nodes.empty()) { state = ParserState::Error; return; }
        auto operand = nodes.top(); nodes.pop();
        nodes.push(createUnaryNode(op, operand));
    } else {
        if(nodes.size() < 2) { state = ParserState::Error; return; }
        auto right = nodes.top(); nodes.pop();
        auto left  = nodes.top(); nodes.pop();
        nodes.push(createBinaryNode(op, left, right));
    }
}

void Parser::processOperatorStack(const std::string& currentOp) {
    while(!ops.empty() && ops.top() != "(" &&
          precedence(ops.top()) >= precedence(currentOp))
        createNodeFromOp();
}

std::shared_ptr<StatementNode> Parser::parseProgram() {
    auto block = std::make_shared<BlockCode>();
    int stmtCount = 0;
    while(currentToken.type != TokenType::EndOfExpr) {
        auto stmt = parseStatement();
        if(stmt) {
            block->addStatement(stmt);
            stmtCount++;
        } 
        else if(state == ParserState::Error) break;
        else nextToken();
    }
    return block;
}

std::shared_ptr<StatementNode> Parser::parseStatement() {
    switch(currentToken.type) {
        case TokenType::If:        return parseIf();
        case TokenType::While:     return parseWhile();
        case TokenType::OpenBrace: return parseBlock();
        case TokenType::Print:     return parsePrint();
        case TokenType::Name:      return parseAssignment();
        case TokenType::For:       return parseFor();
        case TokenType::Function:  return parseFunction();
        case TokenType::Return:    return parseReturn();
        default:
            parseExpression();
            if(currentToken.type == TokenType::Semicolon) nextToken();
            return nullptr;
    }
}

std::shared_ptr<StatementNode> Parser::parseIf() {
    nextToken(); // skip 'if'
    if(currentToken.value != "(") { state = ParserState::Error; return nullptr; }
    nextToken();
    auto cond = parseExpression();
    if(currentToken.value != ")") { state = ParserState::Error; return nullptr; }
    nextToken();
    auto thenBr = parseStatement();
    std::shared_ptr<StatementNode> elseBr = nullptr;
    if(currentToken.type == TokenType::Else) {
        nextToken();
        // Check if this is "else if"
        if(currentToken.type == TokenType::If) {
            // else if -> recursively parse as nested if
            elseBr = parseIf();
        } else {
            elseBr = parseStatement();
        }
    }
    return std::make_shared<IfStatementNode>(cond, thenBr, elseBr);
}

std::shared_ptr<StatementNode> Parser::parseWhile() {
    nextToken(); // skip 'while'
    if(currentToken.value != "(") { state = ParserState::Error; return nullptr; }
    nextToken();
    auto cond = parseExpression();
    if(currentToken.value != ")") { state = ParserState::Error; return nullptr; }
    nextToken();
    auto body = parseStatement();
    return std::make_shared<WhileStatementNode>(cond, body);
}

std::shared_ptr<StatementNode> Parser::parseBlock() {
    auto block = std::make_shared<BlockCode>();
    nextToken(); // skip '{'
    while(currentToken.type != TokenType::CloseBrace &&
          currentToken.type != TokenType::EndOfExpr) {
        block->addStatement(parseStatement());
    }
    if(currentToken.type == TokenType::CloseBrace) nextToken();
    return block;
}

std::shared_ptr<StatementNode> Parser::parseAssignment() {
    std::string name = currentToken.value;
    size_t addr = symTable.getOffset(name);
    nextToken();

    // Function call statement: foo(args);
    if(currentToken.type == TokenType::OpenParen) {
        auto callNode = parseFunctionCall(name);
        if(currentToken.type == TokenType::Semicolon) nextToken();
        return std::make_shared<FunctionCallStatementNode>(callNode);
    }

    if(currentToken.type == TokenType::Assign) {
        nextToken();
        auto expr = parseExpression();
        if(currentToken.type == TokenType::Semicolon) nextToken();
        return std::make_shared<AssignmentNode>(addr, expr);
    }

    if(currentToken.type == TokenType::CompoundAssign) {
        std::string op = currentToken.value; // += , -=, ...
        nextToken();
        auto rhs = parseExpression();
        if(currentToken.type == TokenType::Semicolon) nextToken();
        std::string baseOp = op.substr(0, 1);
        auto varNode = std::make_shared<VariableNode>(addr);
        auto expr = std::make_shared<BinaryOpNode>(baseOp, varNode, rhs);
        return std::make_shared<AssignmentNode>(addr, expr);
    }

    state = ParserState::Error;
    return nullptr;
}

std::shared_ptr<StatementNode> Parser::parsePrint() {
    nextToken(); // skip 'print'
    if(currentToken.value != "(") { state = ParserState::Error; return nullptr; }
    nextToken();
    std::vector<std::shared_ptr<ASTNode>> args;
    while(currentToken.value != ")" && currentToken.type != TokenType::EndOfExpr) {
        if(currentToken.type == TokenType::StringLiteral) {
            args.push_back(std::make_shared<StringNode>(currentToken.value));
            nextToken(); 
        } else {
            args.push_back(parseExpression());
        }
        if(currentToken.type == TokenType::Comma) {
            nextToken();
            if(currentToken.value != ")") {
                args.push_back(std::make_shared<StringNode>()); // Default separator is
            }
        }
    }
    if(currentToken.value != ")") { state = ParserState::Error; return nullptr; }
    nextToken();
    if(currentToken.type == TokenType::Semicolon) nextToken();
    return std::make_shared<PrintNode>(std::move(args));
}

std::shared_ptr<ASTNode> Parser::parsePrimary() {
    Token token = currentToken;
    if (token.type == TokenType::Operator && (token.value == "-" || token.value == "+")) {
        std::string op = token.value;
        nextToken();
        auto child = parsePrimary();
        return createUnaryNode(op == "-" ? "_" : "#", child);
    }
    if (token.type == TokenType::Not) {
        nextToken();
        auto child = parsePrimary();
        return createUnaryNode("not", child);
    }
    if(token.type == TokenType::Number) {
        nextToken();
        return std::make_shared<NumberNode>(std::stod(token.value));
    } else if(token.type == TokenType::Boolean) {
        nextToken();
        double val = (token.value == "true") ? 1.0 : 0.0;
        return std::make_shared<NumberNode>(val);
    } else if(token.type == TokenType::StringLiteral) {
        nextToken();
        return std::make_shared<StringNode>(token.value);
    } else if(token.type == TokenType::Name) {
        std::string name = token.value;
        nextToken();
        if(currentToken.type == TokenType::OpenParen) {
            return parseFunctionCall(name);
        } else {
            size_t offset = symTable.getOffset(name);
            return std::make_shared<VariableNode>(offset);
        }
    } else if(token.type == TokenType::OpenParen) {
        nextToken();
        auto expr = parseExpression();
        if(currentToken.type != TokenType::CloseParen) {
            state = ParserState::Error;
            return nullptr;
        }
        nextToken();
        return expr;
    }
    state = ParserState::Error;
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    std::stack<std::shared_ptr<ASTNode>> localNodes;
    std::stack<std::string> localOps;
    ParserState localState = ParserState::ExpectOperand;

    while (localState != ParserState::Done && localState != ParserState::Error) {
        // Stop tokens
        if (currentToken.type == TokenType::Semicolon  ||
            currentToken.type == TokenType::Comma       ||
            currentToken.type == TokenType::CloseBrace  ||
            currentToken.type == TokenType::Else        ||
            currentToken.type == TokenType::EndOfExpr) {
            localState = ParserState::Done;
            break;
        }

<<<<<<< HEAD
        Token token = currentToken;
        switch(state) {
            case ParserState::ExpectOperand:
                if(token.type == TokenType::Number) {
                    nodes.push(std::make_shared<NumberNode>(std::stod(token.value)));
                    state = ParserState::ExpectOperator;
                    nextToken();
                } else if(token.type == TokenType::Boolean) {
                    double val = (token.value == "true") ? 1.0 : 0.0;
                    nodes.push(std::make_shared<NumberNode>(val));
                    state = ParserState::ExpectOperator;
                    nextToken();
                } else if(token.type == TokenType::Name) {
                    nextToken();
                    if(currentToken.type == TokenType::OpenParen) {
                        auto callNode = parseFunctionCall(token.value);
                        nodes.push(callNode);
                        state = ParserState::ExpectOperator;
                    } else {
                        nodes.push(std::make_shared<VariableNode>(symTable.getAddress(token.value)));
                        state = ParserState::ExpectOperator;
                    }
                } else if(token.type == TokenType::StringLiteral) {
                    nodes.push(std::make_shared<StringNode>(token.value));
                    state = ParserState::ExpectOperator;
                    nextToken();
                } else if(token.type == TokenType::OpenParen) {
                    ops.push("("); 
                    nextToken();
                } else if(token.type == TokenType::Operator &&
                          (token.value == "-" || token.value == "+")) {
                    ops.push(token.value == "-" ? "_" : "#"); nextToken();
                } else if(token.type == TokenType::Not) {
                    ops.push("not"); nextToken();
=======
        if (localState == ParserState::ExpectOperand) {
            auto primary = parsePrimary();
            if (!primary) {
                localState = ParserState::Error;
                break;
            }
            localNodes.push(primary);
            localState = ParserState::ExpectOperator;
        }
        else if (localState == ParserState::ExpectOperator) {
            Token token = currentToken;

            // Binary operator
            if (token.type == TokenType::Operator ||
                token.type == TokenType::CompareOp ||
                token.value == "and" || token.value == "or") {
                // Apply higher precedence operators
                while (!localOps.empty() && localOps.top() != "(" &&
                       precedence(localOps.top()) >= precedence(token.value)) {
                    std::string op = localOps.top(); localOps.pop();
                    if (op == "_" || op == "#" || op == "not") {
                        auto operand = localNodes.top(); localNodes.pop();
                        localNodes.push(createUnaryNode(op, operand));
                    } else {
                        auto right = localNodes.top(); localNodes.pop();
                        auto left = localNodes.top(); localNodes.pop();
                        localNodes.push(createBinaryNode(op, left, right));
                    }
                }
                localOps.push(token.value);
                nextToken();
                localState = ParserState::ExpectOperand;
            }
            // Implicit multiplication: when we see a primary (number, name, string, open paren, not, unary +-)
            else if (token.type == TokenType::Number ||
                     token.type == TokenType::Name ||
                     token.type == TokenType::StringLiteral ||
                     token.type == TokenType::OpenParen ||
                     token.type == TokenType::Not ||
                     (token.type == TokenType::Operator && (token.value == "-" || token.value == "+"))) {
                // Apply higher precedence operators (including '*')
                while (!localOps.empty() && localOps.top() != "(" &&
                       precedence(localOps.top()) >= precedence("*")) {
                    std::string op = localOps.top(); localOps.pop();
                    if (op == "_" || op == "#" || op == "not") {
                        auto operand = localNodes.top(); localNodes.pop();
                        localNodes.push(createUnaryNode(op, operand));
                    } else {
                        auto right = localNodes.top(); localNodes.pop();
                        auto left = localNodes.top(); localNodes.pop();
                        localNodes.push(createBinaryNode(op, left, right));
                    }
                }
                localOps.push("*");
                localState = ParserState::ExpectOperand;
            }
            else if (token.type == TokenType::CloseParen) {
                // Apply all operators until '('
                while (!localOps.empty() && localOps.top() != "(") {
                    std::string op = localOps.top(); localOps.pop();
                    if (op == "_" || op == "#" || op == "not") {
                        auto operand = localNodes.top(); localNodes.pop();
                        localNodes.push(createUnaryNode(op, operand));
                    } else {
                        auto right = localNodes.top(); localNodes.pop();
                        auto left = localNodes.top(); localNodes.pop();
                        localNodes.push(createBinaryNode(op, left, right));
                    }
                }
                if (!localOps.empty() && localOps.top() == "(") {
                    localOps.pop();
>>>>>>> 37c62253fa08934c2bae054db3a95e11c543af6e
                } else {
                    localState = ParserState::Error;
                    break;
                }
                nextToken(); // consume ')'
            }
            else {
                localState = ParserState::Error;
                break;
<<<<<<< HEAD

            case ParserState::ExpectOperator:
                if(token.type == TokenType::Operator ||
                   token.type == TokenType::CompareOp || 
                   token.value == "and" || token.value == "or") {
                    processOperatorStack(token.value);
                    ops.push(token.value);
                    state = ParserState::ExpectOperand; nextToken();
                } else if(token.type == TokenType::StringLiteral) {
                    processOperatorStack("+");
                    ops.push("+");
                    nodes.push(std::make_shared<StringNode>(token.value));
                    state = ParserState::ExpectOperator; 
                    nextToken();
                } else if(token.type == TokenType::CloseParen) {
                    while(!ops.empty() && ops.top() != "(") createNodeFromOp();
                    if(!ops.empty() && ops.top() == "(") {
                        ops.pop();
                        state = ParserState::ExpectOperator; nextToken();
                    } else {
                        state = ParserState::Done;
                    }
                } else if(token.type == TokenType::Name) {
                    nextToken();
                    if(currentToken.type == TokenType::OpenParen) {
                        auto callNode = parseFunctionCall(token.value);
                        nodes.push(callNode);
                        state = ParserState::ExpectOperator;
                    } else {
                        nodes.push(std::make_shared<VariableNode>(symTable.getAddress(token.value)));
                        state = ParserState::ExpectOperator;
                    }
                } else if(token.type == TokenType::Number ||
                          token.type == TokenType::Name   ||
                          token.type == TokenType::OpenParen) {
                    processOperatorStack("*"); ops.push("*");
                    if(token.type == TokenType::Number) {
                        nodes.push(std::make_shared<NumberNode>(std::stod(token.value)));
                        state = ParserState::ExpectOperator; nextToken();
                    } else if(token.type == TokenType::Name) {
                        nodes.push(std::make_shared<VariableNode>(symTable.getAddress(token.value)));
                        state = ParserState::ExpectOperator; nextToken();
                    } else {
                        ops.push("(");
                        state = ParserState::ExpectOperand; nextToken();
                    }
                }
                break;
            default: break;
=======
            }
>>>>>>> 37c62253fa08934c2bae054db3a95e11c543af6e
        }
    }

    // Apply remaining operators
    while (!localOps.empty()) {
        std::string op = localOps.top(); localOps.pop();
        if (op == "_" || op == "#" || op == "not") {
            auto operand = localNodes.top(); localNodes.pop();
            localNodes.push(createUnaryNode(op, operand));
        } else {
            auto right = localNodes.top(); localNodes.pop();
            auto left = localNodes.top(); localNodes.pop();
            localNodes.push(createBinaryNode(op, left, right));
        }
    }

    return localNodes.empty() ? nullptr : localNodes.top();
}

std::shared_ptr<StatementNode> Parser::parseFor() {
    nextToken(); // skip 'for'
    if(currentToken.value != "(") { state = ParserState::Error; return nullptr; }
    nextToken(); // skip '('

    auto init = parseAssignment();

    auto cond = parseExpression();
    if(currentToken.type == TokenType::Semicolon) nextToken();

    auto update = parseAssignment();

    if(currentToken.value != ")") { state = ParserState::Error; return nullptr; }
    nextToken();

    auto body = parseStatement();
    return std::make_shared<ForStatementNode>(init, cond, update, body);
}

std::shared_ptr<StatementNode> Parser::parseFunction() {
    nextToken(); // skip 'function'
    std::string name = currentToken.value;
    nextToken(); // skip function name
    if(currentToken.value != "(") {
        state = ParserState::Error;
        return nullptr;
    }
    nextToken(); // skip '('
    std::vector<std::string> params;
    while(currentToken.value != ")" && currentToken.type != TokenType::EndOfExpr) {
        params.push_back(currentToken.value);
        nextToken();
        if(currentToken.type == TokenType::Comma) nextToken(); // skip ','
    }
    if(currentToken.value != ")") {
        state = ParserState::Error;
        return nullptr;
    }
    nextToken(); // skip ')'
    auto body = parseBlock();
    return std::make_shared<FunctionDefNode>(name, params, body);
}

std::shared_ptr<StatementNode> Parser::parseReturn() {
    nextToken(); // skip 'return'
    std::shared_ptr<ASTNode> expr = nullptr;
    if(currentToken.type != TokenType::Semicolon) {
        expr = parseExpression();
    }
    if(currentToken.type == TokenType::Semicolon) nextToken();
    return std::make_shared<ReturnNode>(expr);
}

std::shared_ptr<ASTNode> Parser::parseFunctionCall(const std::string& name) {
    nextToken(); // skip '('
    std::vector<std::shared_ptr<ASTNode>> args;
    while(currentToken.value != ")" && currentToken.type != TokenType::EndOfExpr) {
        args.push_back(parseExpression());
        if(currentToken.type == TokenType::Comma) nextToken();
    }
    if(currentToken.value != ")") {
        state = ParserState::Error;
        return nullptr;
    }
    nextToken(); // skip ')'
    return std::make_shared<FunctionCallNode>(name, std::move(args));
}

