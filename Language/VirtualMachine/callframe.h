#pragma once
#include "../SymbolTable/symbol_table.h"
#include <vector>

struct CallFrame {
    size_t returnAddress;
    size_t returnDest;
    Value callerSp;
    Value callerFp;
    std::vector<Value> args;
    std::vector<Value> callerRegisters;
};