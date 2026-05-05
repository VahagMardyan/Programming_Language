#pragma once
#include <iostream>
#include <sstream>

class Lexer {
    private:
        std::istream& stream;
        int currentChar;
        int lineNumber = 1;
        int tokenLineNumber = 1;
    public:
        Lexer(std::istream&);
        void advance();
        int peek() const;
        bool isEOF() const;
        int getLineNumber() const {
            return tokenLineNumber;
        }
        void markTokenStart() {
            tokenLineNumber = lineNumber;
        }
};