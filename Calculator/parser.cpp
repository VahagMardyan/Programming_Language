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
        if(op == "/") return std::make_shared<NumberNode>(r != 0 ? l / r : 0);
        long long li = (long long)l, ri = (long long)r;
        if(op == "&")  return std::make_shared<NumberNode>((double)(li & ri));
        if(op == "|")  return std::make_shared<NumberNode>((double)(li | ri));
        if(op == "^")  return std::make_shared<NumberNode>((double)(li ^ ri));
        if(op == "<<") return std::make_shared<NumberNode>((double)(li << ri));
        if(op == ">>") return std::make_shared<NumberNode>((double)(li >> ri));
        if(op == "%")  return std::make_shared<NumberNode>((double)(li % ri));
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
    return std::make_shared<UnaryOpNode>(op, child);
}

int Parser::precedence(const std::string& op) const {
    if(op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=") return 0;
    if(op == "|" || op == "^") return 1;
    if(op == "&") return 2;
    if(op == "<<" || op == ">>") return 3;
    if(op == "+" || op == "-") return 4;
    if(op == "*" || op == "/" || op == "%") return 5;
    if(op == "_" || op == "#") return 6;
    return -1;
}

void Parser::createNodeFromOp() {
    if(ops.empty()) return;
    std::string op = ops.top(); ops.pop();
    if(op == "_" || op == "#") {
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
    while(currentToken.type != TokenType::EndOfExpr) {
        auto stmt = parseStatement();
        if(stmt) block->addStatement(stmt);
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
        elseBr = parseStatement();
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
    size_t addr = symTable.getAddress(name);
    nextToken();
    if(currentToken.type != TokenType::Assign) { state = ParserState::Error; return nullptr; }
    nextToken();
    auto expr = parseExpression();
    if(currentToken.type == TokenType::Semicolon) nextToken();
    return std::make_shared<AssignmentNode>(addr, expr);
}

std::shared_ptr<StatementNode> Parser::parsePrint() {
    nextToken(); // skip 'print'
    if(currentToken.value != "(") { state = ParserState::Error; return nullptr; }
    nextToken();
    std::vector<std::shared_ptr<ASTNode>> args;
    while(currentToken.value != ")" && currentToken.type != TokenType::EndOfExpr) {
        args.push_back(parseExpression());
        if(currentToken.type == TokenType::Comma) nextToken();
    }
    if(currentToken.value != ")") { state = ParserState::Error; return nullptr; }
    nextToken();
    if(currentToken.type == TokenType::Semicolon) nextToken();
    return std::make_shared<PrintNode>(std::move(args));
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    state = ParserState::ExpectOperand;
    nodes = std::stack<std::shared_ptr<ASTNode>>();
    ops   = std::stack<std::string>();

    while(state != ParserState::Done && state != ParserState::Error) {
        if(currentToken.type == TokenType::Semicolon  ||
           currentToken.type == TokenType::Comma       ||
           currentToken.type == TokenType::CloseBrace  ||
           currentToken.type == TokenType::Else) {
            state = ParserState::Done; break;
        }
        if(currentToken.type == TokenType::EndOfExpr) break;

        Token token = currentToken;
        switch(state) {
            case ParserState::ExpectOperand:
                if(token.type == TokenType::Number) {
                    nodes.push(std::make_shared<NumberNode>(std::stod(token.value)));
                    state = ParserState::ExpectOperator; nextToken();
                } else if(token.type == TokenType::Name) {
                    nodes.push(std::make_shared<VariableNode>(symTable.getAddress(token.value)));
                    state = ParserState::ExpectOperator; nextToken();
                } else if(token.type == TokenType::OpenParen) {
                    ops.push("("); nextToken();
                } else if(token.type == TokenType::Operator &&
                          (token.value == "-" || token.value == "+")) {
                    ops.push(token.value == "-" ? "_" : "#"); nextToken();
                } else {
                    state = ParserState::Error;
                }
                break;

            case ParserState::ExpectOperator:
                if(token.type == TokenType::Operator ||
                   token.type == TokenType::CompareOp) {
                    processOperatorStack(token.value);
                    ops.push(token.value);
                    state = ParserState::ExpectOperand; nextToken();
                } else if(token.type == TokenType::CloseParen) {
                    while(!ops.empty() && ops.top() != "(") createNodeFromOp();
                    if(!ops.empty() && ops.top() == "(") {
                        ops.pop();
                        state = ParserState::ExpectOperator; nextToken();
                    } else {
                        state = ParserState::Done;
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
        }
    }
    while(!ops.empty()) createNodeFromOp();
    return nodes.empty() ? nullptr : nodes.top();
}