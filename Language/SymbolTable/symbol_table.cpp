// #include "symbol_table.h"

// size_t SymbolTable::getAddress(const std::string& name) {
//     // Search from innermost scope to outermost
//     for(int i = (int)scopes.size() - 1; i >= 0; i--) {
//         auto it = scopes[i].nameToIndex.find(name);
//         if(it != scopes[i].nameToIndex.end()) {
//             return it->second;
//         }
//     }
    
//     // Not found - create in current (innermost) scope
//     Scope& current = scopes.back();
//     size_t addr = current.startAddress + current.size;
//     current.nameToIndex[name] = addr;
//     current.size++;
    
//     if(addr >= memory.size()) {
//         memory.push_back(0.0);
//     }
    
//     return addr;
// }

// void SymbolTable::setValueByAddress(size_t address, const Value& value) {
//     if(address < memory.size()) memory[address] = value;
// }

// Value SymbolTable::getValueByAddress(size_t address) const {
//     if(address < memory.size()) return memory[address];
//     throw std::runtime_error("Invalid address");
// }

// void SymbolTable::setVariable(const std::string& name, const Value& value) {
//     memory[getAddress(name)] = value;
// }

// Value SymbolTable::getValue(const std::string& name) const {
//     for(int i = (int)scopes.size() - 1; i >= 0; i--) {
//         auto it = scopes[i].nameToIndex.find(name);
//         if(it != scopes[i].nameToIndex.end()) {
//             return memory[it->second];
//         }
//     }
//     throw std::runtime_error("Variable not found: " + name);
// }