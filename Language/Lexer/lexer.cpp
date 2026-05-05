#include "lexer.h"

Lexer::Lexer(std::istream& s) : stream(s) {
    advance();
}
void Lexer::advance() {
    if(currentChar == '\n') {
        lineNumber++;
    }
    currentChar = stream.get();
}

int Lexer::peek() const {
    return currentChar;
}

bool Lexer::isEOF() const {
    return currentChar == EOF;
}

