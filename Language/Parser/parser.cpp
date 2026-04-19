#include "parser.h"
#include <cmath>

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
        if(op == "//") return std::make_shared<NumberNode>(r != 0 ? std::floor(l/r) : 0);
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
        if (op == ">")  return std::make_shared<NumberNode>(l > r ? 1.0 : 0.0);
        if (op == "<")  return std::make_shared<NumberNode>(l < r ? 1.0 : 0.0);
        if (op == ">=") return std::make_shared<NumberNode>(l >= r ? 1.0 : 0.0);
        if (op == "<=") return std::make_shared<NumberNode>(l <= r ? 1.0 : 0.0);
        if (op == "==") return std::make_shared<NumberNode>(l == r ? 1.0 : 0.0);
        if (op == "!=") return std::make_shared<NumberNode>(l != r ? 1.0 : 0.0);
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
    if(op == "*" || op == "/" || op == "%" || op == "//" || op == "%/") return 7;
    if(op == "not" || op == "_" || op == "#") return 8;
    if(op == "**") return 9;
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
    while(!ops.empty() && ops.top() != "(") {
        int topPrec = precedence(ops.top());
        int currentPrec = precedence(currentOp);
        if(topPrec < currentPrec) break;

        if(topPrec == currentPrec) {
            if(currentOp == "**") break;
        }
        createNodeFromOp();
    }
}

std::shared_ptr<StatementNode> Parser::parseProgram() {
    symTable.beginProgramParse();
    auto block = std::make_shared<BlockCode>();
    while(currentToken.type != TokenType::EndOfExpr) {
        auto stmt = parseStatement();
        if(stmt) block->addStatement(stmt);
        else if(state == ParserState::Error) break;
        else nextToken();
    }
    symTable.endProgramParse();
    return block;
}

std::shared_ptr<StatementNode> Parser::parseStatement() {
    switch(currentToken.type) {
        case TokenType::If:        return parseIf();
        case TokenType::While:     return parseWhile();
        case TokenType::OpenBrace: return parseBlock();
        case TokenType::Print:     return parsePrint();
        case TokenType::For:       return parseFor();
        case TokenType::Function:  return parseFunction();
        case TokenType::Return:    return parseReturn();
        case TokenType::Name:      return parseAssignment();

        case TokenType::Local: return parseAssignment();
        case TokenType::Global: return parseAssignment();
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
    
    // Enter new scope for 'then' branch
    symTable.enterBlockScope();
    auto thenBr = parseStatement();
    symTable.exitBlockScope();
    
    std::shared_ptr<StatementNode> elseBr = nullptr;
    if(currentToken.type == TokenType::Else) {
        nextToken();
        // Enter new scope for 'else' branch
        symTable.enterBlockScope();
        elseBr = parseStatement();
        symTable.exitBlockScope();
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
    
    // Enter new scope for while body
    symTable.enterBlockScope();
    auto body = parseStatement();
    symTable.exitBlockScope();
    
    return std::make_shared<WhileStatementNode>(cond, body);
}

std::shared_ptr<StatementNode> Parser::parseBlock() {
    auto block = std::make_shared<BlockCode>();
    nextToken(); // skip '{'
    
    // Enter new scope for this block
    symTable.enterBlockScope();

    while(currentToken.type != TokenType::CloseBrace &&
          currentToken.type != TokenType::EndOfExpr) {
        auto stmt = parseStatement();
        if(stmt) block->addStatement(stmt);
        else if(state == ParserState::Error) {
            break;
        }
    }

    if(currentToken.type == TokenType::CloseBrace) {
        nextToken();
    }
    
    // Exit block scope
    symTable.exitBlockScope();
    
    return block;
}

std::shared_ptr<StatementNode> Parser::parseAssignment() {
    bool explicitLocal = false;
    bool explicitGlobal = false;
    std::string name;

    if (currentToken.type == TokenType::Local) {
        explicitLocal = true;
        nextToken();
    } 
    else if (currentToken.type == TokenType::Global) {
        explicitGlobal = true;
        nextToken();
    }

    if (currentToken.type != TokenType::Name) {
        state = ParserState::Error;
        return nullptr;
    }

    name = currentToken.value;
    nextToken();

    if (currentToken.type == TokenType::OpenParen) {
        auto callNode = parseFunctionCall(name);
        if (currentToken.type == TokenType::Semicolon) nextToken();
        return std::make_shared<FunctionCallStatementNode>(callNode);
    }

    bool isLocalVar = false;

    if (explicitLocal) {
        isLocalVar = true;
    } 
    else if (explicitGlobal) {
        isLocalVar = false;
    } 
    else if (insideFunction) {
        // Default to local when inside any function
        isLocalVar = true;
    } 
    else {
        isLocalVar = false;
    }

    if (isLocalVar) {
        symTable.getLocalOffset(name);
    } else {
        symTable.getGlobalAddress(name);
    }

    if(currentToken.type != TokenType::Assign && 
       currentToken.type != TokenType::CompoundAssign) {
        state = ParserState::Error;
        return nullptr;
    }

    std::string assignOp = currentToken.value;
    nextToken();

    auto valueExpr = parseExpression();

    if(currentToken.type == TokenType::Semicolon) {
        nextToken();
    }

    if(assignOp != "=") {
        std::string mathOp;
        if(assignOp == "+=") mathOp = "+";
        else if(assignOp == "-=") mathOp = "-";
        else if(assignOp == "*=") mathOp = "*";
        else if(assignOp == "/=") mathOp = "/";
        else if(assignOp == "%=") mathOp = "%";
        else if(assignOp == "^=") mathOp = "^";

        std::shared_ptr<ASTNode> varNode;
        if (isLocalVar) {
            int32_t off = symTable.getLocalOffset(name);
            varNode = std::make_shared<VariableNode>(off);
        } else {
            size_t addr = symTable.getGlobalAddress(name);
            varNode = std::make_shared<VariableNode>(addr);
        }

        valueExpr = std::make_shared<BinaryOpNode>(mathOp, varNode, valueExpr);
    }

    if (isLocalVar) {
        int32_t off = symTable.getLocalOffset(name);
        return std::make_shared<AssignmentNode>(off, valueExpr);
    } else {
        size_t addr = symTable.getGlobalAddress(name);
        return std::make_shared<AssignmentNode>(addr, valueExpr);
    }
}

std::shared_ptr<StatementNode> Parser::parsePrint() {
    nextToken(); // skip 'print'
    if(currentToken.value != "(") { state = ParserState::Error; return nullptr; }
    nextToken(); // skip '('

    std::vector<std::shared_ptr<ASTNode>> exprs;
    while(currentToken.value != ")" && currentToken.type != TokenType::EndOfExpr) {
        exprs.push_back(parseExpression());
        if(currentToken.type == TokenType::Comma) nextToken();
    }

    if(currentToken.value != ")") { state = ParserState::Error; return nullptr; }
    nextToken(); // skip ')'

    if(exprs.size() <= 1) {
        exprs.push_back(std::make_shared<StringNode>("\n"));
    }

    if(currentToken.type == TokenType::Semicolon) {
        nextToken();
    }

    return std::make_shared<PrintNode>(std::move(exprs));
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    state = ParserState::ExpectOperand;
    while(!ops.empty()) ops.pop();
    while(!nodes.empty()) nodes.pop();

    while(true) {
        Token token = currentToken;
        
        if(token.type == TokenType::EndOfExpr ||
           token.type == TokenType::Semicolon ||
           token.type == TokenType::Comma ||
           (token.type == TokenType::CloseParen && ops.empty())) {
            break;
        }

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
                    std::string name = token.value;
                    nextToken();
                    if(currentToken.type == TokenType::OpenParen) {
                        auto callNode = parseFunctionCall(name);
                        nodes.push(callNode);
                        state = ParserState::ExpectOperator;
                    } else {
                        if (symTable.isLocal(name)) {
                            int32_t off = symTable.getLocalOffset(name);
                            nodes.push(std::make_shared<VariableNode>(off));
                        } else {
                            size_t addr = symTable.getGlobalAddress(name);
                            nodes.push(std::make_shared<VariableNode>(addr));
                        }
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
                } else {
                    state = ParserState::Error;
                }
                break;

            case ParserState::ExpectOperator:
                if(token.type == TokenType::Operator ||
                   token.type == TokenType::CompareOp || 
                   token.value == "and" || token.value == "or") {
                    processOperatorStack(token.value);
                    ops.push(token.value);
                    state = ParserState::ExpectOperand; 
                    nextToken();
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
                    std::string name = token.value;
                    nextToken();
                    if(currentToken.type == TokenType::OpenParen) {
                        auto callNode = parseFunctionCall(name);
                        nodes.push(callNode);
                        state = ParserState::ExpectOperator;
                    } else {
                        if (symTable.isLocal(name)) {
                            int32_t off = symTable.getLocalOffset(name);
                            nodes.push(std::make_shared<VariableNode>(off));
                        } else {
                            size_t addr = symTable.getGlobalAddress(name);
                            nodes.push(std::make_shared<VariableNode>(addr));
                        }
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
                        std::string name = token.value;
                        nextToken();
                        if (symTable.isLocal(name)) {
                            int32_t off = symTable.getLocalOffset(name);
                            nodes.push(std::make_shared<VariableNode>(off));
                        } else {
                            size_t addr = symTable.getGlobalAddress(name);
                            nodes.push(std::make_shared<VariableNode>(addr));
                        }
                        state = ParserState::ExpectOperator;
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

std::shared_ptr<StatementNode> Parser::parseFor() {
    nextToken(); // skip 'for'
    if(currentToken.value != "(") { state = ParserState::Error; return nullptr; }
    nextToken(); // skip '('

    // Enter new scope for the entire for loop
    // This makes the loop variable and body variables all local to the for loop
    symTable.enterBlockScope();

    // i = 0;
    auto init = parseAssignment();

    // i <= 10;
    auto cond = parseExpression();
    if(currentToken.type == TokenType::Semicolon) nextToken();

    // i += 1;
    auto update = parseAssignment();

    if(currentToken.value != ")") { state = ParserState::Error; return nullptr; }
    nextToken();

    // {...}
    auto body = parseStatement();
    
    // Exit the for loop scope
    symTable.exitBlockScope();
    
    return std::make_shared<ForStatementNode>(init, cond, update, body);
}


std::shared_ptr<StatementNode> Parser::parseFunction() {
    nextToken(); // skip 'function'
    std::string name = currentToken.value;
    nextToken(); // skip function name

    if (currentToken.value != "(") {
        state = ParserState::Error;
        return nullptr;
    }
    nextToken(); // skip '('

    std::vector<std::string> params;
    while (currentToken.value != ")" && currentToken.type != TokenType::EndOfExpr) {
        if (currentToken.type != TokenType::Name) {
            state = ParserState::Error;
            return nullptr;
        }
        params.push_back(currentToken.value);
        nextToken();
        if (currentToken.type == TokenType::Comma) nextToken();
    }

    if (currentToken.value != ")") {
        state = ParserState::Error;
        return nullptr;
    }
    nextToken(); // skip ')'

    insideFunction = true;
    symTable.enterFunctionScope();  // This creates the first scope level

    // Register parameters in the function's first scope
    for (const auto& p : params) {
        symTable.getLocalOffset(p);
    }

    if (currentToken.type != TokenType::OpenBrace) {
        state = ParserState::Error;
        return nullptr;
    }

    auto body = parseBlock();  // parseBlock will manage its own nested scope

    int slotCount = symTable.getLocalSlotCountForFrame();
    symTable.exitFunctionScope();
    insideFunction = false;

    return std::make_shared<FunctionDefNode>(name, params, body, slotCount);
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

std::shared_ptr<StatementNode> Parser::parseReturn() {

    nextToken(); // skip 'return'

    std::shared_ptr<ASTNode> expr = nullptr;
    if (currentToken.type != TokenType::Semicolon && 
        currentToken.type != TokenType::EndOfExpr &&
        currentToken.type != TokenType::CloseBrace) {
        expr = parseExpression();
    }

    if (currentToken.type == TokenType::Semicolon) {
        nextToken();
    }

    return std::make_shared<ReturnNode>(expr);
}