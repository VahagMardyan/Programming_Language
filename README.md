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

### Linux / macOS (g++ / clang)
```bash
g++ -std=c++20 -O3 *.cpp -o vhg
# or with static linking (Linux)
g++ -std=c++20 -O3 -static *.cpp -o vhg
```

### Windows (MSVC Developer Command Prompt)
```cmd
cl /EHsc /O2 /std:c++20 *.cpp /Fe:vhg.exe
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
| Bitwise           | `&` `\|` `^` `<<` `>>`                                  |
| Logical           | `and` `or` `not`                                        |
| Comparison        | `==` `!=` `<` `>` `<=` `>=`                             |
| Assignment        | `=` `+=` `-=` `*=` `/=` `%=` `^=`                       |
| String            | `+` (concatenation), `length(s)` -> s.size()            |
| Mathematical Constants | `m_pi` -> π, `m_e` -> e                            |
| Built-in mathematical functions | 
                                        |`sin`, `cos`, `tan`, `asin`,
                                        `acos`, `atan`, `atan2`, `sqrt`,
                                        `cbrt`, `pow`, `exp`, `log`, `ln`,
                                        `log10`, `log2`, `log_ab`, `fmod`,
                                        `ceil`, `floor`, `abs`, `round`|

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

> **Note:** VHG doesn't support recursive functions yet. Please use loop-iterative versions instead of recursion.

### Built‑in I/O
- `print(expr1, expr2, ...)` – prints each argument; automatically appends a newline **if only one argument is given** (otherwise you must include `"\n"` explicitly).
- `length(string)` - returns the size of given string.
---

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

---

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
