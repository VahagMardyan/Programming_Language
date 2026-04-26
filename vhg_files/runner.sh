#!/bin/bash

set -e

SOURCES="../Language/AST/ast.cpp ../Language/Compiler/compiler.cpp ../Language/Lexer/lexer.cpp ../Language/Parser/parser.cpp ../Language/Runner/main.cpp ../Language/Tokenizer/tokenizer.cpp ../Language/VirtualMachine/vm.cpp"

CXX="g++"
CXXFLAGS="-std=c++20 -O3"

echo "Compiling VHG..."

$CXX $CXXFLAGS $SOURCES -o vhg

if [ $? -eq 0 ]; then
    echo ""
    echo "[OK] Build successful!"
    echo ""
    echo "Usage:"
    echo "  ./vhg program.vhg"
    echo "  ./vhg compile input.vhg [output.vhb]"
    echo "  ./vhg run program.vhb [--debug]"
else
    echo ""
    echo "[ERROR] Build failed!"
    echo "Check the error messages above."
fi