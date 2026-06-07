#!/bin/bash

set -e

SOURCES="../Language/AST/ast.cpp ../Language/Compiler/compiler.cpp ../Language/Lexer/lexer.cpp ../Language/Parser/parser.cpp ../Language/Runner/main.cpp ../Language/Tokenizer/tokenizer.cpp ../Language/VirtualMachine/vm.cpp ../Language/VirtualMachine/debugger.cpp ../Language/VirtualMachine/printer.cpp ..Language/Linker/linker.cpp"

CXX="g++"
CXXFLAGS="-std=c++20 -O3"

echo "Compiling VHG..."

$CXX $CXXFLAGS $SOURCES -o vhg

if [ $? -eq 0 ]; then
    echo ""
    echo "[OK] Build successful!"
    echo ""
    echo "Usage:"
    echo "./vhg.exe <file.vhg>                               Build and run source directly"
    echo "./vhg.exe comple <in.vhg> [out.vhb]                Compile to bytecode"
    echo "./vhg.exe compile-obj <in.vhg> [out.vhb]           Compile to linkable object"
    echo "./vhg.exe link <a.vhb> [b.vhb ...] -o out.vhb       Link objects"
    echo "./vhg.exe run <in.vhb> [--debug]                    Run bytecode"
else
    echo ""
    echo "[ERROR] Build failed!"
    echo "Check the error messages above."
fi