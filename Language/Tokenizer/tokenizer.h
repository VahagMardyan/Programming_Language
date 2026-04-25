#pragma once
#include "../Lexer/lexer.h"
#include <unordered_map>

enum class TokenType {
    Number, Name, Operator, OpenParen, CloseParen, EndOfExpr, Error,
    If, Else, While, For,
    OpenBrace, CloseBrace, // {, }
    Semicolon, // ;
    CompareOp, // == != > < >= <=
    Assign, // =
    Comma, // ,
    Print, // print function
    StringLiteral, // string
    And, // &&
    Or, // ||
    Not, // !
    CompoundAssign, // +=, -=, *=, /=, %= and ^=
    Boolean, // true/false
    Function, // function,
    Return, // return value
    Global, // global variable
    Local, // local variable
    Void, // for void functions
    Math_const_vars, // constants PI, E
    QuestionMark, // ?
    Colon, // :
    None, // none
    Break, // break;
    Continue, // continue;
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