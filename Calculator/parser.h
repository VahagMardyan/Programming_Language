#pragma once
#include <stack>
#include <memory>
#include <string>

#include "tokenizer.h"
#include "ast.h"
#include "symbol_table.h"

enum class ParserState {
    ExpectOperand,
    ExpectOperator,
    Done,
    Error
};

class Parser {
    private:
        Tokenizer& tokenizer;
        ParserState state;
        SymbolTable& symTable;
        std::stack<std::string> ops;

        std::stack<std::shared_ptr<ASTNode>> nodes;
        int precedence(const std::string& op) const;
        void processOperatorStack(const std::string& currentOp);
        void createNodeFromOp();
    public:
        Parser(Tokenizer& tok, SymbolTable& st);
        std::shared_ptr<ASTNode> parse();
        
};