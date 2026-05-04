# 🧪 VHG Language Compiler & Virtual Machine

**VHG** is a small, self‑contained programming language that compiles to a custom **register‑based bytecode** and runs on a **virtual machine**. The entire toolchain is written in modern C++ and demonstrates a complete compiler pipeline: lexical analysis, recursive‑descent parsing with operator precedence, an abstract syntax tree (AST), constant folding optimizations, and a RISC‑inspired instruction set.

---

## 🚀 Highlights

- **Full compiler pipeline** – Lexer → Tokenizer → Parser → AST → Compiler → Bytecode → VM.
- **Rich language features** – variables (global/local), block scoping, `if`/`else`, `while`, `for` loops, functions with parameters and return values.
- **Strong typing for numbers and strings** – arithmetic, bitwise, logical, and comparison operators; string concatenation.
- **Optimizations** – constant folding, implicit multiplication, post‑order code generation.
- **Standalone bytecode** – binary `.vhb` files with a `VHB1` magic header, loadable and executable by the VM without re‑parsing.
- **Clean, modular C++20** – extensive use of standard library, smart pointers, and RAII.

---

## 📦 Project Structure

| Directory / File         | Purpose                                                                 |
|--------------------------|-------------------------------------------------------------------------|
| `main.cpp`               | CLI entry point – compile, run, or directly execute `.vhg` files.        |
| `Lexer/`                 | Character‑by‑character input stream handling.                            |
| `Tokenizer/`             | Converts characters into tokens (keywords, operators, literals).         |
| `SymbolTable/`           | Manages variable scopes, stack offsets, and global addresses.            |
| `AST/`                   | AST node definitions and the `OpCode` enumeration.                       |
| `Parser/`                | Recursive‑descent parser with shunting‑yard expression handling.         |
| `Compiler/`              | Transforms AST into bytecode; performs constant folding.                 |
| `VirtualMachine/`        | Executes bytecode; includes register file, memory, and call stack.       |

---

## 🛠️ Building the Compiler

### Requirements
- C++20 compatible compiler (g++ ≥ 11, clang ≥ 14, or MSVC 2022).
- Standard library with filesystem support.
- Simply run `runner.{ext}` on your OS.

### Linux / macOS (g++ / clang)
```bash
g++ -std=c++20 -O3 *.cpp -o vhg
# or with static linking (Linux)
g++ -std=c++20 -O3 -static *.cpp -o vhg
```

```sh (runner.sh)
#!/bin/bash

set -e

SOURCES="AST/ast.cpp Compiler/compiler.cpp Lexer/lexer.cpp Parser/parser.cpp Runner/main.cpp Tokenizer/tokenizer.cpp VirtualMachine/vm.cpp"

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
    echo "  ./vhg compile input.vhg output.vhb"
    echo "  ./vhg run program.vhb [--debug]"
else
    echo ""
    echo "[ERROR] Build failed!"
    echo "Check the error messages above."
fi
```

```shell
chmod +x runner.sh
./runner.sh
```

### Windows (MSVC Developer Command Prompt)
```cmd
cl /EHsc /O2 /std:c++20 *.cpp /Fe:vhg.exe
```

```batch (runner.bat)
@echo off
setlocal enabledelayedexpansion

set SOURCES=AST\ast.cpp Compiler\compiler.cpp Lexer\lexer.cpp Parser\parser.cpp Runner\main.cpp Tokenizer\tokenizer.cpp VirtualMachine\vm.cpp

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
    echo   ./vhg.exe compile input.vhg output.vhb
    echo   ./vhg.exe run program.vhb [--debug]
) else (
    echo.
    echo [ERROR] Build failed!
    echo Check the error messages above.
)

endlocal
```

```cmd
./runner.bat
```

> **Note:** The project does not depend on any external libraries beyond the C++ standard library.

---

## 🏃 Usage

The executable `vhg` (or `vhg.exe`) accepts three modes:

### 1. Run source directly (backward‑compatible)
```bash
./vhg program.vhg
```
Parses, compiles, and executes the `.vhg` source in one step.

### 2. Compile to bytecode
```bash
./vhg compile input.vhg [output.vhb]
```
If no output path is given, it defaults to `input.vhb`.

### 3. Run pre‑compiled bytecode
```bash
./vhg run program.vhb
```
Loads the `.vhb` binary and executes it on the VM.

---

## 📝 Language Syntax Overview

### Variables & Scoping
- **Global** variables persist throughout the program.
- **Local** variables are declared inside blocks (including function bodies) and use stack‑based allocation.
- Use the `local` or `global` keyword to explicitly control storage; otherwise, the parser defaults to **local** inside any block and **global** at the top level.

```vhg
global counter = 0;          # explicit global
local  temp    = 42;         # explicit local

for (i = 0; i < 10; i += 1) {
    local square = i * i;    # block‑scoped local
    print(square, "\n");
}
```

### Data Types
- **Numbers** – double‑precision floating point (internally `double`).
- **Strings** – double‑ or single‑quoted literals; supports escape sequences `\n`, `\t`, `\"`, `\\`.
- **Booleans** – `true` and `false` are stored as `1.0` and `0.0`.

### Operators
| Category          | Operators                                               |
|-------------------|---------------------------------------------------------|
| Arithmetic        | `+` `-` `*` `/` `%` `//` (floor) `%/` (fractional) `**` |
| Bitwise           | `&` `\|` `^` `<<` `>>` `~`                              |
| Logical           | `and` `or` `not`                                        |
| Comparison        | `==` `!=` `<` `>` `<=` `>=`                             |
| Assignment        | `=` `+=` `-=` `*=` `/=` `%=` `^=`                       |
| String            | `+` `+=` (concatenation), `length(s)` -> s.size(), `*` -> string multiplication       |
| Ternary           | `condition ? trueBranch : falseBranch` (e.g `x = 5 > 6 ? 7 : 8;`)         |
| Loop operators    | `break;` -> exit loop earlier, `continue;` -> skip next iteration |

### Mathematical Functions

| Function | Description | Number of Arguments |
|----------|-------------|---------------------|
| `sin(x)` | Sine | 1 |
| `cos(x)` | Cosine | 1 |
| `tan(x)` | Tangent | 1 |
| `asin(x)` | Arc sine | 1 |
| `acos(x)` | Arc cosine | 1 |
| `atan(x)` | Arc tangent | 1 |
| `atan2(y, x)` | Arc tangent (two arguments) | 2 |
| `sqrt(x)` | Square root | 1 |
| `cbrt(x)` | Cube root | 1 |
| `pow(x, y)` | Power (x^y) | 2 |
| `exp(x)` | Exponential (e^x) | 1 |
| `log(x)` | Natural logarithm (base e) | 1 |
| `ln(x)` | Natural logarithm (base e) | 1 |
| `log10(x)` | Base-10 logarithm | 1 |
| `log2(x)` | Base-2 logarithm | 1 |
| `log_ab(a, b)` | Logarithm of `b` with base `a` (log(b)/log(a)) | 2 |
| `ceil(x)` | Round up | 1 |
| `floor(x)` | Round down | 1 |
| `round(x)` | Round to nearest integer | 1 |
| `abs(x)` | Absolute value | 1 |
| `fmod(x, y)` | Floating-point remainder | 2 |

### Mathematical Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `m_pi` | 3.141592653589793 | π (Pi) |
| `m_e`  | 2.718281828459045 | e (Euler's number) |

### Control Flow
```vhg
if (x > 0) {
    print("positive\n");
} else if (x < 0) {
    print("negative\n");
} else {
    print("zero\n");
}

while (n > 0) {
    n -= 1;
}

for (i = 0; i < 5; i += 1) {
    print(i, " ");
}
```

### Functions
```vhg
# # non-void function requires "return"
function add(a, b) {
    return a + b;
}

void function foo() {
    print("Hello world");
}

result = add(10, 20);
foo();
print(result);
```

- Parameters are passed by value.
- Functions can be called before they are defined (forward declaration via bytecode patching).

<!-- > **Note:** VHG doesn't support recursive functions yet. Please use loop-iterative versions instead of recursion. -->

### Switch/case
```
x = 2;
switch(x) {
    case 1:
        print("One\n");
        break;
    case 2, 3, 4:
        print("Two, Three or Four\n");
        break;
    case 5:
        print("Five\n");
        break;
    default:
        print("Other\n");
}
```
### Built‑in I/O
- `input(prompt)` - User Input.
- `print(expr1, expr2, ...)` – prints each argument; automatically appends a newline **if only one argument is given** (otherwise you must include `"\n"` explicitly).
- `length(string)` - returns the size of given string.
- `type(argument)` - Returns the type of given argument (`string`, `number` or `none`).
- `chr(number)` - Return an ASCII string of one character with ordinal i; 0 <= i <= 255.
- `ord(string)` - Return an ASCII code point for a one-character string.
- `bin(integer)` - Return the binary representation of an integer.
- `oct(integer)` - Return the octal representation of an integer.
- `hex(integer)` - Return the hexadecimal representation of an integer.
- `dec(string)` - Returns the decimal representation of given argment (if possible).
---

|   Syntax  |   Example   | Will understand as           |
|   `0b`, `0B`    |   `0b1100`   | BIN                   |
|   `0o`, `0O`    |   `0o45`    | OCT                    |
|   `0x`, `0X`     |   `0xff`    | Hex                   |

>**Note:** Negative numbers are written with a leading `-`, e.g. `"-0b1100"` is `-12`. \n
>**Note:** `bin(num)`, `oct(num)` and `hex(num)` where `num < 0` return the two's complement of `num` (32-bit).

### Import preprocessing
`import "path_to_file"`
- This allows to import functions and variables (global) from other files.

## 🧠 Architecture Deep Dive

### Lexer & Tokenizer
- `Lexer` provides a stream interface with `peek()` and `advance()`.
- `Tokenizer` groups characters into tokens, skipping whitespace and comments (`# ...` and `#* ... *#`).

### Parser
- Recursive descent for statements (`if`, `while`, `for`, `function`, `return`, blocks).
- **Shunting‑Yard algorithm** for expressions, respecting operator precedence and associativity.
- Implicit multiplication (e.g., `2x` or `(a+b)(c+d)`) is handled by injecting a `*` token when appropriate.
- **Constant folding** is performed *during parsing* to simplify the AST immediately.

### Symbol Table
- Manages nested block scopes via a stack of `ScopeLevel` objects.
- Global variables are stored in a flat address space.
- Local variables receive negative offsets relative to the **frame pointer** (`FP` / `x8`).
- Function definitions push a fresh scope stack, preserving outer scopes for later restoration.
- Declared but unassigned variables store `none` by default.
```vhg
x;
print(x); # # The output will be: none
```

### Compiler
- Traverses the AST in post‑order, generating a linear sequence of `Instruction`s.
- Allocates virtual registers on‑the‑fly (except `x2` = SP, `x8` = FP).
- Emits function prologues/epilogues that adjust SP and FP.
- Patches forward function calls after all code is generated.
- **Constant folding** is re‑applied during optimization (redundant constants are merged).
- Outputs a `ByteCode` structure containing instructions, constant pool (numbers), and string pool.

### Bytecode Format (`.vhb`)
| Offset | Field               | Size            |
|--------|---------------------|-----------------|
| 0      | Magic `"VHB1"`      | 4 bytes         |
| 4      | Instruction count   | 4 bytes (uint32)|
| 8      | Constant count      | 4 bytes (uint32)|
| 12     | String count        | 4 bytes (uint32)|
| 16     | Instructions        | `count * 4` bytes (op, dst, left, right each 1 byte) |
| …      | Constants           | `count * 8` bytes (double) |
| …      | Strings             | each: length (uint32) + UTF‑8 data |

### Virtual Machine
- **Register file** – 256+ registers (indexed by `uint8_t`), with `x2` as stack pointer and `x8` as frame pointer.
- **Memory** – linear array of `Value` (variant of double and string).
- **Call stack** – saves return address, caller’s SP/FP, and argument buffer.
- **Instruction set** – includes RISC‑V inspired arithmetic (`ADD`, `SUB`, `AND`, …), control flow (`JMP`, `JZ`, `CALL`, `RETURN`), and memory access (`LOAD`/`STORE` relative to FP).
- Debug mode (`VirtualMachine(true)`) prints the AST and a disassembly of the generated bytecode.

## 📊 Example Program
```vhg

# Loop and local scoping
sum = 0;
for (i = 1; i <= 10; i += 1) {
    local square = i * i;
    sum += square;
}
print("Sum of squares 1..10 = ", sum, "\n");
```

Run it:
```bash
./vhg compile fact.vhg
./vhg run fact.vhb
```
---

### Debug Mode
To run your program step by step with the built-in debugger:
```bash
./vhg compile app.vhg
./vhg run app.bin --debug
```

The debugger will
- Display all generated bytecode instructions
- Pause before each instruction
- Show current register values (non-zero)
- Accept commands
    * `Enter` - execute next instruction (step)
    * `c` - continue (run to end)
    * `q` - quit debugger
    * `r<n>` - print value of register (e.g., `r0`)
    * `m<addr>` - print value of memory address (e.g., `m10000`)
---
