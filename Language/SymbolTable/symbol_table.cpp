#include "symbol_table.h"

size_t SymbolTable::getAddress(const std::string& name) {
    auto it = nameToIndex.find(name);
    if(it != nameToIndex.end()) return it -> second;
    size_t addr = memory.size();
    nameToIndex[name] = addr;
    memory.push_back(0.0);
    return addr;
}

void SymbolTable::setValueByAddress(size_t address, const Value& value) {
    if(address < memory.size()) memory[address] = value;
}

Value SymbolTable::getValueByAddress(size_t address) const {
    if(address < memory.size()) return memory[address];
    throw std::runtime_error("Invalid address");
}

void SymbolTable::setVariable(const std::string& name, const Value& value) {
    memory[getAddress(name)] = value;
}

Value SymbolTable::getValue(const std::string& name) const {
    auto it = nameToIndex.find(name);
    if(it != nameToIndex.end()) return memory[it -> second];
    throw std::runtime_error("Variable not found: " + name);
}