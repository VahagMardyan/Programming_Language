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
        if(op == "/") {
            if(r == 0) return std::make_shared<BinaryOpNode>(op, left, right);
            return std::make_shared<NumberNode>(l / r);
        }
        if(op == "//") {
            if(r == 0) return std::make_shared<BinaryOpNode>(op, left, right);
            return std::make_shared<NumberNode>(std::floor(l/r));
        }
        long long li = (long long)l, ri = (long long)r;
        if(op == "&")  return std::make_shared<NumberNode>((double)(li & ri));
        if(op == "|")  return std::make_shared<NumberNode>((double)(li | ri));
        if(op == "^")  return std::make_shared<NumberNode>((double)(li ^ ri));
        if(op == "<<") return std::make_shared<NumberNode>((double)(li << ri));
        if(op == ">>") return std::make_shared<NumberNode>((double)((uint32_t)li >> (ri & 0x1F)));
        if(op == "%")  {
            if(ri == 0) return std::make_shared<BinaryOpNode>(op, left, right);
            return std::make_shared<NumberNode>((double)(li % ri));
        }
        if(op == "%/") {
            if(r == 0) return std::make_shared<BinaryOpNode>(op, left, right);
            return std::make_shared<NumberNode>(l / r - std::floor(l / r));
        }
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

std::shared_ptr<ASTNode> Parser::resolveVariableNode(const std::string& name) {
    int32_t localOffset = 0;
    if (symTable.tryGetLocalOffset(name, localOffset)) {
        return std::make_shared<VariableNode>(localOffset);
    }

    size_t globalAddr = 0;
    if (symTable.tryGetGlobalAddress(name, globalAddr)) {
        return std::make_shared<VariableNode>(globalAddr);
    }

    state = ParserState::Error;
    throw std::runtime_error("Undefined variable: " + name);
}

bool Parser::shouldDefaultToLocal(bool explicitGlobal) const {
    return !explicitGlobal && symTable.hasActiveScope();
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
        case TokenType::Void:      return parseFunction();
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
    else {
        isLocalVar = shouldDefaultToLocal(explicitGlobal);
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
        int32_t localOffset = 0;
        size_t globalAddr = 0;

        if (explicitLocal) {
            if (!symTable.tryGetLocalOffset(name, localOffset)) {
                state = ParserState::Error;
                throw std::runtime_error("Undefined local variable in compound assignment: " + name);
            }
            isLocalVar = true;
        } else if (explicitGlobal) {
            if (!symTable.tryGetGlobalAddress(name, globalAddr)) {
                state = ParserState::Error;
                throw std::runtime_error("Undefined global variable in compound assignment: " + name);
            }
            isLocalVar = false;
        } else if (symTable.tryGetLocalOffset(name, localOffset)) {
            isLocalVar = true;
        } else if (symTable.tryGetGlobalAddress(name, globalAddr)) {
            isLocalVar = false;
        } else {
            state = ParserState::Error;
            throw std::runtime_error("Undefined variable in compound assignment: " + name);
        }

        std::string mathOp;
        if(assignOp == "+=")      mathOp = "+";
        else if(assignOp == "-=") mathOp = "-";
        else if(assignOp == "*=") mathOp = "*";
        else if(assignOp == "/=") mathOp = "/";
        else if(assignOp == "%=") mathOp = "%";
        else if(assignOp == "^=") mathOp = "^";

        std::shared_ptr<ASTNode> varNode;
        if (isLocalVar) {
            varNode = std::make_shared<VariableNode>(localOffset);
        } else {
            varNode = std::make_shared<VariableNode>(globalAddr);
        }

        valueExpr = std::make_shared<BinaryOpNode>(mathOp, varNode, valueExpr);
    } else {
        if (isLocalVar) {
            symTable.getLocalOffset(name);
        } else {
            symTable.getGlobalAddress(name);
        }
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

std::shared_ptr<ASTNode> Parser::parseBuiltInCall(const std::string& name) {
    if(name == "length") {
        nextToken(); // skip '('
        
        // Save parser state before parseExpression()
        auto savedOps = ops;
        auto savedNodes = nodes;
        auto savedState = state;
        
        auto arg = parseExpression();
        
        // Restore parser state after recursive call
        ops = savedOps;
        nodes = savedNodes;
        state = savedState;
        
        if(!arg) return nullptr;
        if(currentToken.value != ")") {
            state = ParserState::Error;
            return nullptr;
        }
        nextToken(); // skip ')'
        return std::make_shared<LengthNode>(arg);
    }
    // future built-ins
    state = ParserState::Error;
    return nullptr;
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
           token.type == TokenType::Colon ||
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
                            std::shared_ptr<ASTNode> node;
                            if(name == "length") { // or any other built-in
                                node = parseBuiltInCall(name);
                            } else {
                                node = parseFunctionCall(name);
                            }
                            if(!node) return nullptr;
                            nodes.push(node);
                            state = ParserState::ExpectOperator;
                        } else {
                            nodes.push(resolveVariableNode(name));
                            state = ParserState::ExpectOperator;
                        }
                } else if(token.type == TokenType::StringLiteral) {
                    nodes.push(std::make_shared<StringNode>(token.value));
                    state = ParserState::ExpectOperator;
                    nextToken();
                }  else if(token.type == TokenType::Math_const_vars) {
                    OpCode constOp = (token.value == "m_pi") ? OpCode::CONST_PI : OpCode::CONST_E;
                    nodes.push(std::make_shared<MathConstantNode>(constOp));
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
                if(token.type == TokenType::QuestionMark) {
                    while(!ops.empty() && ops.top() != "(") {
                        createNodeFromOp();
                    }
                    if(state == ParserState::Error || nodes.empty()) {
                        state = ParserState::Error;
                        return nullptr;
                    }
                    nextToken(); // skip "?"
                    auto savedOps = std::move(ops);
                    auto savedNodes = std::move(nodes);
                    auto trueExpr = parseExpression();
                    ops = std::move(savedOps);
                    nodes = std::move(savedNodes);
                    if(currentToken.type != TokenType::Colon) {
                        state = ParserState::Error;
                        return nullptr;
                    }
                    nextToken(); // skip ":"
                    savedOps = std::move(ops);
                    savedNodes = std::move(nodes);
                    auto falseExpr = parseExpression();
                    ops = std::move(savedOps);
                    nodes = std::move(savedNodes);
                    if(!trueExpr || !falseExpr || nodes.empty()) {
                        state = ParserState::Error;
                        return nullptr;
                    }
                    auto cond = nodes.top(); nodes.pop();
                    nodes.push(std::make_shared<TernaryOpNode>(cond, trueExpr, falseExpr));
                    state = ParserState::ExpectOperator;
                    break;
                } else if(token.type == TokenType::Operator ||
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
                    } else if (ops.empty()) {
                        break;
                    } else {
                        state = ParserState::Error;
                    }
                } else if(token.type == TokenType::Number ||
                          token.type == TokenType::Name   ||
                          token.type == TokenType::OpenParen ||
                          token.type == TokenType::StringLiteral) {
                    processOperatorStack("*"); ops.push("*");
                    if(token.type == TokenType::Number) {
                        nodes.push(std::make_shared<NumberNode>(std::stod(token.value)));
                        state = ParserState::ExpectOperator; nextToken();
                        } else if(token.type == TokenType::Name) {
                            std::string name = token.value;
                            nextToken();
                            if(currentToken.type == TokenType::OpenParen) {
                                std::shared_ptr<ASTNode> node;
                                if(name == "length") {
                                    node = parseBuiltInCall(name);
                                } else {
                                    node = parseFunctionCall(name);
                                }
                                if(!node) return nullptr;
                                nodes.push(node);
                            } else {
                                nodes.push(resolveVariableNode(name));
                            }
                            state = ParserState::ExpectOperator;
                        } else if(token.type == TokenType::StringLiteral) {
                            nodes.push(std::make_shared<StringNode>(token.value));
                            state = ParserState::ExpectOperator;
                            nextToken();
                    } else {
                        ops.push("(");
                        state = ParserState::ExpectOperand; nextToken();
                    }
                } else {
                    state = ParserState::Error;
                }
                break;
            default: break;
        }
        if(state == ParserState::Error) break;
    }
    while(state != ParserState::Error && !ops.empty()) {
        if(ops.top() == "(") {
            state = ParserState::Error;
            break;
        }
        createNodeFromOp();
    }
    if(state == ParserState::Error || nodes.size() != 1) {
        state = ParserState::Error;
        return nullptr;
    }
    return nodes.top();
}

std::shared_ptr<StatementNode> Parser::parseFor() {
    nextToken(); // skip 'for'
    if(currentToken.value != "(") { state = ParserState::Error; return nullptr; }
    nextToken(); // skip '('

    // Enter new scope for the entire for loop.
    symTable.enterBlockScope();

    // i = 0;
    auto init = parseAssignment();

    // i <= 10;
    auto cond = parseExpression();
    if(currentToken.type == TokenType::Semicolon) nextToken();

    // i += 1;
    auto update = parseAssignment();

    if(currentToken.value != ")") {
        state = ParserState::Error;
        symTable.exitBlockScope();
        return nullptr;
    }
    nextToken();

    // {...}
    auto body = parseStatement();

    // Exit the for loop scope
    symTable.exitBlockScope();
    
    return std::make_shared<ForStatementNode>(init, cond, update, body);
}

std::shared_ptr<StatementNode> Parser::parseFunction() {

    bool isVoid = false;
    if (currentToken.type == TokenType::Void) {
        isVoid = true;
        nextToken();   // skip 'void'
    }

    if (currentToken.type != TokenType::Function) {
        state = ParserState::Error;
        throw std::runtime_error("Expected 'function' keyword");
    }
    nextToken(); // skip 'function'

    if (currentToken.type != TokenType::Name) {
        state = ParserState::Error;
        throw std::runtime_error("Expected function name");
    }
    std::string name = currentToken.value;
    nextToken();

    if (currentToken.value != "(") {
        state = ParserState::Error;
        throw std::runtime_error("Expected '(' after function name");
    }
    nextToken(); // skip '('

    std::vector<std::string> params;
    while (currentToken.value != ")" && currentToken.type != TokenType::EndOfExpr) {
        if (currentToken.type != TokenType::Name) {
            state = ParserState::Error;
            throw std::runtime_error("Expected parameter name");
        }
        params.push_back(currentToken.value);
        nextToken();
        if (currentToken.type == TokenType::Comma) {
            nextToken();
        }
    }

    if (currentToken.value != ")") {
        state = ParserState::Error;
        throw std::runtime_error("Expected ')' after parameters");
    }
    nextToken(); // skip ')'

    if (currentToken.type != TokenType::OpenBrace) {
        state = ParserState::Error;
        throw std::runtime_error("Expected '{' before function body");
    }

    insideFunction = true;
    symTable.enterFunctionScope();

    for (const auto& p : params) {
        symTable.getLocalOffset(p);
    }

    auto body = parseBlock();

    if (!isVoid) {

        auto block = std::dynamic_pointer_cast<BlockCode>(body);
        if (!block) {
            symTable.exitFunctionScope();
            insideFunction = false;
            state = ParserState::Error;
            throw std::runtime_error("Function body is not a block");
        }

        const auto& statements = block->getStatements();
        bool hasReturn = false;
        if (!statements.empty()) {
            if (std::dynamic_pointer_cast<ReturnNode>(statements.back())) {
                hasReturn = true;
            }
        }
        if (!hasReturn) {
            symTable.exitFunctionScope();
            insideFunction = false;
            state = ParserState::Error;
            throw std::runtime_error("Non-void function '" + name + "' must end with a return statement");
        }
    }

    int slotCount = symTable.getLocalSlotCountForFrame();

    symTable.exitFunctionScope();
    insideFunction = false;

    return std::make_shared<FunctionDefNode>(name, params, body, slotCount, isVoid);
}

std::shared_ptr<ASTNode> Parser::parseFunctionCall(const std::string& name) {
    nextToken(); // skip '('
    std::vector<std::shared_ptr<ASTNode>> args;
    auto savedOps = ops;
    auto savedNodes = nodes;
    auto savedState = state;
    while(currentToken.value != ")" && currentToken.type != TokenType::EndOfExpr) {
        auto arg = parseExpression();
        if (state == ParserState::Error || !arg) {
            return nullptr;
        }
        args.push_back(arg);
        ops = savedOps;
        nodes = savedNodes;
        state = savedState;
        if(currentToken.type == TokenType::Comma) nextToken();
    }
    if(currentToken.value != ")") {
        state = ParserState::Error;
        return nullptr;
    }
    ops = savedOps;
    nodes = savedNodes;
    state = savedState;
    nextToken(); // skip ')'
    return std::make_shared<FunctionCallNode>(name, std::move(args));
}

std::shared_ptr<StatementNode> Parser::parseReturn() {
    if (!insideFunction) {
        state = ParserState::Error;
        throw std::runtime_error("return is only allowed inside functions");
    }

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