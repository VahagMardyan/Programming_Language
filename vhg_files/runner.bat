@echo off

@REM Clear all stale Visual Studio environment variables
set "VSINSTALLDIR="
set "VCINSTALLDIR="
set "VCToolsVersion="
set "VCToolsRedistVersion="
set "VSCMD_ARG_VCVARS_VER="
set "VSCMD_VER="

@REM Call Build Tools environment directly
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

setlocal enabledelayedexpansion

set SOURCES=..\Language\AST\ast.cpp ..\Language\Compiler\compiler.cpp ..\Language\Lexer\lexer.cpp ..\Language\Parser\parser.cpp ..\Language\Runner\main.cpp ..\Language\Tokenizer\tokenizer.cpp ..\Language\VirtualMachine\vm.cpp ..\Language\VirtualMachine\debugger.cpp ..\Language\VirtualMachine\printer.cpp ..\Language\Linker\linker.cpp

set CXX=cl
set CXXFLAGS=/EHsc /O2 /std:c++20 /W3

echo Compiling VHG...

%CXX% %CXXFLAGS% %SOURCES% /Fe:vhg.exe

if %errorlevel% equ 0 (
    echo.
    echo [OK] Build successful!
    echo.
    echo Usage:
    echo "./vhg.exe <file.vhg>                             Build and run source directly"
    echo "./vhg.exe compile <in.vhg> [out.vhb]             Compile to bytecode"
    echo "./vhg.exe compile-obj <in.vhg> [out.vhb]         Compile to linkable object"
    echo "./vhg.exe link <a.vhb> [b.vhb ...] -o out.vhb      Link objects"
    echo "./vhg.exe run <in.vhb> [--debug]                  Run bytecode"
) else (
    echo.
    echo [ERROR] Build failed!
    echo Check the error messages above.
)

endlocal