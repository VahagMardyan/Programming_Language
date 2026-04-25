#include "tokenizer.h"
#include <algorithm>
#include <cctype>
#include <unordered_set>

const std::unordered_map<std::string, TokenType> operations = {
    {"+", TokenType::Operator}, {"-", TokenType::Operator},
    {"*", TokenType::Operator}, {"/", TokenType::Operator},
    {"&", TokenType::Operator}, {"|", TokenType::Operator},
    {"^", TokenType::Operator}, {"%", TokenType::Operator},
    {">>", TokenType::Operator}, {"<<", TokenType::Operator},
    {"**", TokenType::Operator}, {"//", TokenType::Operator},
    {"%/", TokenType::Operator},
};

namespace {
    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return s;
    }

    const std::unordered_set<std::string> BuiltIns = {
        "sin", "cos", "tan",
        "asin", "acos", "atan", "atan2",
        "sqrt", "exp", "log", "ln", "log10",
        "ceil", "floor", "abs", "round",
        "fmod", "cbrt", "log2", "pow", "log_ab", // log(b)/log(a)
        "input", // user-input
    };
}

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
    if(current == '?') { lexer.advance(); return {TokenType::QuestionMark, "?"}; }
    if(current == ':') { lexer.advance(); return {TokenType::Colon, ":"}; }

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
        const std::string lowered = toLower(name);
        if(lowered == "if")    return {TokenType::If,    lowered};
        if(lowered == "else")  return {TokenType::Else,  lowered};
        if(lowered == "while") return {TokenType::While, lowered};
        if(lowered == "for")   return {TokenType::For,   lowered};
        if(lowered == "print") return {TokenType::Print, lowered};
        if(lowered == "and")   return {TokenType::And,   lowered};
        if(lowered == "or")    return {TokenType::Or,    lowered};
        if(lowered == "not")   return {TokenType::Not,   lowered};
        if(lowered == "true" || lowered == "false") return {TokenType::Boolean, lowered};
        if(lowered == "function") return {TokenType::Function, lowered};
        if(lowered == "return") return {TokenType::Return, lowered};
        if(lowered == "local") return {TokenType::Local, lowered};
        if(lowered == "global") return {TokenType::Global, lowered};
        if(lowered == "void") return {TokenType::Void, lowered};
        if(lowered == "m_pi" || lowered == "m_e") return {TokenType::Math_const_vars, lowered};
        if(BuiltIns.find(lowered) != BuiltIns.end()) return {TokenType::Name, lowered};
        if(lowered == "none") return { TokenType::None, lowered };
        if(lowered == "break") return { TokenType::Break, lowered };
        if(lowered == "continue") return { TokenType::Continue, lowered };
        return {TokenType::Name, name};
    }

    // 6. Operators
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
           (current == '>' && next == '>') || // >>
           (current == '+' && next == '=') || // +=
           (current == '-' && next == '=') || // -=
           (current == '/' && next == '=') || // /=
           (current == '*' && next == '=') || // *=
           (current == '%' && next == '=') || // %=
           (current == '^' && next == '=') ||
           (current == '*' && next == '*') || // **
           (current == '/' && next == '/') || // floor division operator (//) 
           (current == '%' && next == '/')) { // fractional division operator (a%/b = a/b - a//b). Fractional part
            op += next;
            lexer.advance();
        }
    }
    if(op == "=")  return {TokenType::Assign, op};
    if(op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=") {
        return {TokenType::CompareOp, op};
    }
    
    if(op == "+=" || op == "-=" || op == "/=" || op == "*=" || op == "%=" || op == "^=") {
        return {TokenType::CompoundAssign, op};
    }
    if(isOperator(op)) return {TokenType::Operator, op};

    return {TokenType::Error, op};
}