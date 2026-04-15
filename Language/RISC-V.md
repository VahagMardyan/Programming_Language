## RISC-V Base Instructions (32-bit)

### Arithmetic Instructions

| Instruction | OpCode | Description |
|-------------|--------|-------------|
| `ADD rd, rs1, rs2` | 0 | rd = rs1 + rs2 (integer addition) |
| `SUB rd, rs1, rs2` | 1 | rd = rs1 - rs2 (integer subtraction) |
| `MUL rd, rs1, rs2` | 2 | rd = rs1 * rs2 (integer multiplication) |
| `DIV rd, rs1, rs2` | 3 | rd = rs1 / rs2 (integer division) |
| `AND rd, rs1, rs2` | 4 | rd = rs1 & rs2 (bitwise AND) |
| `OR rd, rs1, rs2`  | 5 | rd = rs1 \| rs2 (bitwise OR) |
| `XOR rd, rs1, rs2` | 6 | rd = rs1 ^ rs2 (bitwise XOR) |
| `MOD rd, rs1, rs2` | 7 | rd = rs1 % rs2 (remainder) |
| `SLL rd, rs1, rs2` | 8 | rd = rs1 << rs2 (logical shift left) |
| `SRL rd, rs1, rs2` | 9 | rd = rs1 >> rs2 (logical shift right) |

### Immediate Arithmetic Instructions

| Instruction | OpCode | Description |
|-------------|--------|-------------|
| `ADDI rd, rs1, imm` | 10 | rd = rs1 + sign_extend(imm) |
| `SUBI rd, rs1, imm` | 11 | rd = rs1 - sign_extend(imm) |
| `MULI rd, rs1, imm` | 12 | rd = rs1 * sign_extend(imm) |
| `DIVI rd, rs1, imm` | 13 | rd = rs1 / sign_extend(imm) |
| `ANDI rd, rs1, imm` | 14 | rd = rs1 & sign_extend(imm) |
| `ORI rd, rs1, imm`  | 15 | rd = rs1 \| sign_extend(imm) |
| `XORI rd, rs1, imm` | 16 | rd = rs1 ^ sign_extend(imm) |
| `MODI rd, rs1, imm` | 17 | rd = rs1 % sign_extend(imm) |
| `SLLI rd, rs1, imm` | 18 | rd = rs1 << imm |
| `SRLI rd, rs1, imm` | 19 | rd = rs1 >> imm |

### Load/Store Instructions

| Instruction | OpCode | Description |
|-------------|--------|-------------|
| `LW rd, offset(rs1)` | 20 | rd = memory[rs1 + offset] (load word) |
| `SW rs2, offset(rs1)`| 21 | memory[rs1 + offset] = rs2 (store word) |

### Branch Instructions

| Instruction | OpCode | Description |
|-------------|--------|-------------|
| `BEQ rs1, rs2, offset` | 22 | if rs1 == rs2: pc += offset (branch equal) |
| `BNE rs1, rs2, offset` | 23 | if rs1 != rs2: pc += offset (branch not equal) |
| `BLT rs1, rs2, offset` | 24 | if rs1 < rs2: pc += offset (branch less than) |
| `BGE rs1, rs2, offset` | 25 | if rs1 >= rs2: pc += offset (branch greater equal) |

### Jump Instructions

| Instruction | OpCode | Description |
|-------------|--------|-------------|
| `JAL rd, offset` | 26 | rd = pc + 4; pc += offset (jump and link) |
| `JALR rd, rs1, offset` | 27 | rd = pc + 4; pc = rs1 + offset (jump and link register) |

## Extended Instructions (VM-specific)

| Instruction | OpCode | Description |
|-------------|--------|-------------|
| `LOAD_CONST rd, idx` | 28 | rd = constant_pool[idx] (load constant) |
| `LOAD_STR rd, idx` | 29 | rd = string_pool[idx] (load string) |
| `PRINT rs1` | 30 | print(rs1) to stdout |
| `PRINT_STR idx` | 31 | print(string_pool[idx]) to stdout |

### Comparison Instructions (set on condition)

| Instruction | OpCode | Description |
|-------------|--------|-------------|
| `CMP_EQ rd, rs1, rs2` | 32 | rd = (rs1 == rs2) ? 1 : 0 |
| `CMP_NE rd, rs1, rs2` | 33 | rd = (rs1 != rs2) ? 1 : 0 |
| `CMP_LT rd, rs1, rs2` | 34 | rd = (rs1 < rs2) ? 1 : 0 |
| `CMP_GT rd, rs1, rs2` | 35 | rd = (rs1 > rs2) ? 1 : 0 |
| `CMP_LE rd, rs1, rs2` | 36 | rd = (rs1 <= rs2) ? 1 : 0 |
| `CMP_GE rd, rs1, rs2` | 37 | rd = (rs1 >= rs2) ? 1 : 0 |

### Logical Instructions

| Instruction | OpCode | Description |
|-------------|--------|-------------|
| `LOGICAL_AND rd, rs1, rs2` | 38 | rd = (rs1 && rs2) ? 1 : 0 (short-circuit) |
| `LOGICAL_OR rd, rs1, rs2` | 39 | rd = (rs1 \|\| rs2) ? 1 : 0 (short-circuit) |
| `LOGICAL_NOT rd, rs1` | 40 | rd = !rs1 ? 1 : 0 |

### Control Instructions

| Instruction | OpCode | Description |
|-------------|--------|-------------|
| `HALT` | 41 | Stop execution and return |
| `POW rd, rs1, rs2` | 42 | rd = pow(rs1, rs2) (floating-point power) |

## RISC-V Register Convention

| Register | ABI Name | Description | Caller/Callee |
|----------|----------|-------------|---------------|
| x0 | zero | Hardwired zero | - |
| x1 | ra | Return address | Caller |
| x2 | sp | Stack pointer | Callee |
| x5-x7 | t0-t2 | Temporary registers | Caller |
| x8 | fp | Frame pointer | Callee |
| x10-x11 | a0-a1 | Arguments / Return value | Caller |
| x12-x17 | a2-a7 | Arguments | Caller |
| x18-x27 | s2-s11 | Saved registers | Callee |
| x28-x31 | t3-t6 | Temporary registers | Caller |

## Function Call Convention

1. **Arguments**: First 8 arguments passed in a0..a7
2. **Return value**: Stored in a0
3. **Stack frame**: Allocated by callee using `ADDI sp, sp, -frame_size`
4. **Return address**: Stored in ra (x1)
5. **Frame pointer**: fp (x8) points to current frame base

## Example Function Prologue/Epilogue

```asm
# Prologue
ADDI sp, sp, -frame_size   # allocate stack frame
SW ra, offset(sp)          # save return address
SW fp, offset(sp)          # save frame pointer
ADDI fp, sp, frame_size    # set new frame pointer

# ... function body ...

# Epilogue
LW ra, offset(sp)          # restore return address
LW fp, offset(sp)          # restore frame pointer
ADDI sp, sp, frame_size    # deallocate stack frame
JALR zero, ra, 0           # return
```