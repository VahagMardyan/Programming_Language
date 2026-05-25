@echo off
setlocal enabledelayedexpansion

set SOURCES=AST\ast.cpp Compiler\compiler.cpp Lexer\lexer.cpp Parser\parser.cpp Runner\main.cpp Tokenizer\tokenizer.cpp VirtualMachine\vm.cpp VirtualMachine\debugger.cpp VirtualMachine\printer.cpp

set CXX=cl
set CXXFLAGS=/EHsc /O2 /std:c++20 /W3

echo Compiling VHG...

%CXX% %CXXFLAGS% %SOURCES% /Fe:vhg.exe

if %errorlevel% equ 0 (
    echo.
    echo [OK] Build successful!
    echo.
    echo Usage:
    echo   ./vhg.exe program.vhg
    echo   ./vhg.exe compile input.vhg [output.vhb]
    echo   ./vhg.exe run program.vhb
) else (
    echo.
    echo [ERROR] Build failed!
    echo Check the error messages above.
)

endlocal