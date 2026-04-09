#include "tokenizer.h"

const std::unordered_map<std::string, TokenType> operations = {
    {"+", TokenType::Operator}, {"-", TokenType::Operator},
    {"*", TokenType::Operator}, {"/", TokenType::Operator},
    {"&", TokenType::Operator}, {"|", TokenType::Operator},
    {"^", TokenType::Operator}, {"%", TokenType::Operator},
    {">>", TokenType::Operator}, {"<<", TokenType::Operator},
};

Tokenizer::Tokenizer(Lexer& l) : lexer(l) {}

bool Tokenizer::isOperator(const std::string& s) const {
    return operations.find(s) != operations.end();
}

Token Tokenizer::getNextToken() {

    while (!lexer.isEOF()) {
        char current = static_cast<char>(lexer.peek());

        if (isspace(current)) {
            lexer.advance();
            continue;
        }

        if (current == '#') {
            lexer.advance(); // Skip '#'

            if (!lexer.isEOF() && lexer.peek() == '*') {
                lexer.advance(); // Skip '*'
        
                bool foundEnd = false;
                while (!lexer.isEOF()) {
                    if (lexer.peek() == '*') {
                        lexer.advance(); // Skip '*'
                        if (!lexer.isEOF() && lexer.peek() == '#') {
                            lexer.advance(); // Skip '#'
                            foundEnd = true;
                            break;
                        }
                    } else {
                        lexer.advance();
                    }
                }
            } else {
                while (!lexer.isEOF() && lexer.peek() != '\n') {
                    lexer.advance();
                }
            }
            continue;
        }
        break;
    }

    if(lexer.isEOF()) {
        return {TokenType::EndOfExpr, ""};
    }

    while(!lexer.isEOF() && isspace(lexer.peek())) {
        lexer.advance();
    }
    if(lexer.isEOF()) {
        return {
            TokenType::EndOfExpr, ""
        };
    }
    char current = static_cast<char>(lexer.peek());

    if(current == '(') {
        lexer.advance();
        return {
            TokenType::OpenParen, "("
        };
    }
    if(current == ')') {
        lexer.advance();
        return {
            TokenType::CloseParen, ")"
        };
    }
    if(current == '{') {
        lexer.advance();
        return {
            TokenType::OpenBrace, "{"
        };
    }
    if(current == '}') {
        lexer.advance();
        return {
            TokenType::CloseBrace, "}"
        };
    }
    if(current == ';') {
        lexer.advance();
        return {
            TokenType::Semicolon, ";"
        };
    }
    if(current == ',') {
        lexer.advance();
        return {
            TokenType::Comma, ","
        };
    }

    if(isdigit(current) || current == '.') {
        std::string val;
        bool hasDot = false;
        while(!lexer.isEOF() && (isdigit(lexer.peek()) || lexer.peek() == '.') ) {
            if(lexer.peek() == '.') {
                if(hasDot) break;
                hasDot = true;
            }
            val += (char)lexer.peek();
            lexer.advance();
        }
        return {
            TokenType::Number, val
        };
    }
    if(isalnum(current) || current == '_') {
        std::string name;
        while(!lexer.isEOF() && ( isalnum(lexer.peek()) || lexer.peek() == '_')) {
            name += (char)lexer.peek();
            lexer.advance();
        }
        if (name == "if") return {TokenType::If, name};
        if (name == "else") return {TokenType::Else, name};
        if (name == "while") return {TokenType::While, name};
        if (name == "print") return {TokenType::Print, name};
        return {
            TokenType::Name, name
        };
    }

    std::string op;
    op += current;
    lexer.advance();

    if(!lexer.isEOF()) {
        char next = static_cast<char>(lexer.peek());
        if((current == '=' && next == '=') || // ==
           (current == '!' && next == '=') || // !=
           (current == '<' && next == '=') || // <=
           (current == '>' && next == '=') || // >=
           (current == '<' && next == '<') || // <<
           (current == '>' && next == '>')) { // >>
            op+=next;
            lexer.advance();
        }
    }

    if(op == "=") {
        return {TokenType::Assign, op};
    }

    if(op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=") {
        return {
            TokenType::CompareOp, op
        };
    }

    if(isOperator(op)) {
        return {
            TokenType::Operator, op
        };
    }

    return {
        TokenType::Error, op
    };    
}