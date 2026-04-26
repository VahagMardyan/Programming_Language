@echo off
setlocal enabledelayedexpansion

set SOURCES=..\Language\AST\ast.cpp ..\Language\Compiler\compiler.cpp ..\Language\Lexer\lexer.cpp ..\Language\Parser\parser.cpp ..\Language\Runner\main.cpp ..\Language\Tokenizer\tokenizer.cpp ..\Language\VirtualMachine\vm.cpp

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
    echo   ./vhg.exe run program.vhb [--debug]
) else (
    echo.
    echo [ERROR] Build failed!
    echo Check the error messages above.
)

endlocal