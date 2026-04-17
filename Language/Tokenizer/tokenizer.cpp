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
    // 1. Skip whitespace and comments
    while(!lexer.isEOF()) {
        char current = static_cast<char>(lexer.peek());
        if(isspace(current)) { lexer.advance(); continue; }
        if(current == '#') {
            lexer.advance();
            if(!lexer.isEOF() && lexer.peek() == '*') {
                lexer.advance();
                while(!lexer.isEOF()) {
                    if(lexer.peek() == '*') {
                        lexer.advance();
                        if(!lexer.isEOF() && lexer.peek() == '#') { lexer.advance(); break; }
                    } else { lexer.advance(); }
                }
            } else {
                while(!lexer.isEOF() && lexer.peek() != '\n') lexer.advance();
            }
            continue;
        }
        break;
    }

    if(lexer.isEOF()) return {TokenType::EndOfExpr, ""};

    char current = static_cast<char>(lexer.peek());

    // 2. String literal
    if(current == '"' || current == '\'') {
        char openQuote = current;
        lexer.advance();
        std::string str;
        while(!lexer.isEOF() && lexer.peek() != openQuote) {
            char c = (char)lexer.peek();
            if(c == '\\') {
                lexer.advance();
                char esc = (char)lexer.peek();
                switch(esc) {
                    case 'n':  str += '\n'; break;
                    case 't':  str += '\t'; break;
                    case '"':  str += '"';  break;
                    case '\\': str += '\\'; break;
                    default:   str += esc;  break;
                }
            } else { str += c; }
            lexer.advance();
        }
        if(!lexer.isEOF()) lexer.advance();
        return {TokenType::StringLiteral, str};
    }

    // 3. Punctuation
    if(current == '(') { lexer.advance(); return {TokenType::OpenParen,  "("}; }
    if(current == ')') { lexer.advance(); return {TokenType::CloseParen,  ")"}; }
    if(current == '{') { lexer.advance(); return {TokenType::OpenBrace,   "{"}; }
    if(current == '}') { lexer.advance(); return {TokenType::CloseBrace,  "}"}; }
    if(current == ';') { lexer.advance(); return {TokenType::Semicolon,   ";"}; }
    if(current == ',') { lexer.advance(); return {TokenType::Comma,       ","}; }

    // 4. Numbers
    if(isdigit(current) || current == '.') {
        std::string val;
        bool hasDot = false;
        while(!lexer.isEOF() && (isdigit(lexer.peek()) || lexer.peek() == '.')) {
            if(lexer.peek() == '.') { if(hasDot) break; hasDot = true; }
            val += (char)lexer.peek();
            lexer.advance();
        }
        return {TokenType::Number, val};
    }

    // 5. Keywords and identifiers
    if(isalpha(current) || current == '_') {
        std::string name;
        while(!lexer.isEOF() && (isalnum(lexer.peek()) || lexer.peek() == '_')) {
            name += (char)lexer.peek();
            lexer.advance();
        }
        if(name == "if")    return {TokenType::If,    name};
        if(name == "else")  return {TokenType::Else,  name};
        if(name == "while") return {TokenType::While, name};
        if(name == "for")   return {TokenType::For,   name};
        if(name == "print") return {TokenType::Print, name};
        if(name == "and")   return {TokenType::And,   name};
        if(name == "or")    return {TokenType::Or,    name};
        if(name == "not")   return {TokenType::Not,   name};
        if(name == "true" || name == "false") return {TokenType::Boolean, name};
        if(name == "function") return {TokenType::Function, name};
        if(name == "return") return {TokenType::Return, name};
        return {TokenType::Name, name};
    }

    // 6. Operators
    std::string op;
    op += current;
    lexer.advance();
    if(!lexer.isEOF()) {
        char next = static_cast<char>(lexer.peek());
        if((current == '=' && next == '=') ||
           (current == '!' && next == '=') ||
           (current == '<' && next == '=') ||
           (current == '>' && next == '=') ||
           (current == '<' && next == '<') ||
           (current == '>' && next == '>') ||
           (current == '+' && next == '=') ||
           (current == '-' && next == '=') ||
           (current == '/' && next == '=') ||
           (current == '*' && next == '=') ||
           (current == '%' && next == '=') ||
           (current == '^' && next == '=')) {
            op += next;
            lexer.advance();
        }
    }
    if(op == "=")  return {TokenType::Assign, op};
    if(op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=") {
        return {TokenType::CompareOp, op};
    }
    
    if(op == "+=" || op == "-=" || op == "/=" || op == "*=" || op == "%=") {
        return {TokenType::CompoundAssign, op};
    }
    if(isOperator(op)) return {TokenType::Operator, op};

    return {TokenType::Error, op};
}