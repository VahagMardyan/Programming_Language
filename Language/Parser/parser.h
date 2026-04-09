#pragma once
#include <stack>
#include <memory>
#include <string>
#include "../Tokenizer/tokenizer.h"
#include "../AST/ast.h"
#include "../SymbolTable/symbol_table.h"

enum class ParserState { ExpectOperand, ExpectOperator, Done, Error };

class Parser {
    Tokenizer& tokenizer;
    SymbolTable& symTable;
    Token currentToken;
    std::stack<std::string> ops;
    std::stack<std::shared_ptr<ASTNode>> nodes;
    ParserState state;

    void nextToken() { currentToken = tokenizer.getNextToken(); }
    int precedence(const std::string& op) const;
    void processOperatorStack(const std::string& currentOp);
    void createNodeFromOp();
    std::shared_ptr<ASTNode> createBinaryNode(const std::string& op,
        std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right);
    std::shared_ptr<ASTNode> createUnaryNode(const std::string& op,
        std::shared_ptr<ASTNode> child);
public:
    Parser(Tokenizer& tok, SymbolTable& st)
        : tokenizer(tok), symTable(st), state(ParserState::ExpectOperand) { nextToken(); }

    std::shared_ptr<StatementNode> parseProgram();
    std::shared_ptr<StatementNode> parseStatement();
    std::shared_ptr<StatementNode> parseIf();
    std::shared_ptr<StatementNode> parseWhile();
    std::shared_ptr<StatementNode> parseBlock();
    std::shared_ptr<StatementNode> parseAssignment();
    std::shared_ptr<StatementNode> parsePrint();
    std::shared_ptr<ASTNode> parseExpression();
};