#pragma once
#include "lexer.h"
#include <unordered_map>

enum class TokenType {
    Number, Name, Operator, OpenParen, CloseParen, EndOfExpr, Error
};

struct Token {
    TokenType type;
    std::string value;
};

class Tokenizer {
    private:
        Lexer& lexer;
        bool isOperator(const std::string& s) const;
        static const std::unordered_map<std::string, TokenType> opMap;
    public:
        Tokenizer(Lexer&);
        Token getNextToken();
        
};