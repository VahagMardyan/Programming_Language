#include "linker.h"
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

void Linker::addUnit(ByteCode bc, const std::string& name) {
    units_.push_back({std::move(bc), name});
}

ByteCode Linker::link() {
    if (units_.empty()) {
        throw std::runtime_error("Linker: no translation units to link");
    }

    // Pass 1 – Build unified constant / string / global pools and collect the

    std::vector<double>      mergedConsts;
    std::vector<std::string> mergedStrings;
    size_t                   mergedGlobalSlotCount = 0;
    std::vector<std::string> mergedGlobalNames;

    std::vector<std::vector<int>>    remapConst;
    std::vector<std::vector<int>>    remapStr;
    std::vector<std::vector<size_t>> remapGlobal;

    mergeConstants(units_, mergedConsts,  remapConst);
    mergeStrings  (units_, mergedStrings, remapStr);
    mergeGlobals  (units_, mergedGlobalSlotCount, mergedGlobalNames, remapGlobal);

    // Compute the instruction base offset for each unit in the final stream.
    std::vector<size_t> base(units_.size());
    size_t cursor = 0;
    for (size_t u = 0; u < units_.size(); ++u) {
        base[u] = cursor;
        cursor += units_[u].bc.instructions.size();
    }
    const size_t totalInstructions = cursor;

    // Collect the global function-symbol table with rebased addresses.
    // Detect duplicate definitions.
    std::unordered_map<std::string, size_t> funcTable; // name -> absolute PC
    for (size_t u = 0; u < units_.size(); ++u) {
        for (auto& [name, addr] : units_[u].bc.functionSymbols) {
            size_t absAddr = base[u] + addr;
            if (funcTable.count(name)) {
                throw std::runtime_error(
                    "Linker: duplicate definition of function '" + name + "'"
                    + (units_[u].name.empty() ? "" : " in unit '" + units_[u].name + "'"));
            }
            funcTable[name] = absAddr;
        }
    }

    // Pass 2 – Merge and patch instructions.

    std::vector<Instruction> merged;
    merged.reserve(totalInstructions);
    std::vector<int> mergedLineNums;
    mergedLineNums.reserve(totalInstructions);

    for (size_t u = 0; u < units_.size(); ++u) {
        const ByteCode& bc = units_[u].bc;
        size_t unitBase    = base[u];

        for (size_t i = 0; i < bc.instructions.size(); ++i) {
            Instruction inst = bc.instructions[i];
            const uint8_t op = static_cast<uint8_t>(inst.op);

            // Patch jump / call addresses (absolute within merged stream)
            if (isJumpOrCall(op)) {
                uint16_t oldAddr = readAddr(inst);
                // CALL with oldAddr == 0 means it was an unresolved forward call;
                // we'll fix those below via unresolvedCalls.  Skip rebasing here
                // so we don't corrupt the sentinel.
                bool isUnresolved = (op == static_cast<uint8_t>(OpCode::CALL) && oldAddr == 0);
                if (!isUnresolved) {
                    writeAddr(inst, static_cast<uint16_t>(unitBase + oldAddr));
                }
            }

            // Patch LOAD_CONST: remap constant-pool index
            if (op == static_cast<uint8_t>(OpCode::LOAD_CONST)) {
                int oldIdx = static_cast<int>(inst.left) | (static_cast<int>(inst.right) << 8);
                if (oldIdx < static_cast<int>(remapConst[u].size())) {
                    int newIdx = remapConst[u][oldIdx];
                    inst.left  = static_cast<uint8_t>(newIdx & 0xFF);
                    inst.right = static_cast<uint8_t>((newIdx >> 8) & 0xFF);
                }
            }

            // Patch LOAD_STR / PRINT_STR: remap string-pool index
            if (op == static_cast<uint8_t>(OpCode::LOAD_STR) ||
                op == static_cast<uint8_t>(OpCode::PRINT_STR)) {
                int oldIdx = static_cast<int>(inst.left) | (static_cast<int>(inst.right) << 8);
                if (oldIdx < static_cast<int>(remapStr[u].size())) {
                    int newIdx = remapStr[u][oldIdx];
                    inst.left  = static_cast<uint8_t>(newIdx & 0xFF);
                    inst.right = static_cast<uint8_t>((newIdx >> 8) & 0xFF);
                }
            }

            // Patch LOAD_VAR / STORE_VAR: remap global-slot address
            if (op == static_cast<uint8_t>(OpCode::LOAD_VAR)) {
                // left+right hold the slot address (left = addr & 0xFF, right = addr >> 8)
                size_t oldSlot = static_cast<size_t>(inst.left) |
                                 (static_cast<size_t>(inst.right) << 8);
                if (oldSlot < remapGlobal[u].size()) {
                    size_t newSlot = remapGlobal[u][oldSlot];
                    inst.left  = static_cast<uint8_t>(newSlot & 0xFF);
                    inst.right = static_cast<uint8_t>((newSlot >> 8) & 0xFF);
                }
            }
            if (op == static_cast<uint8_t>(OpCode::STORE_VAR)) {
                
                size_t oldSlot = static_cast<size_t>(inst.left) |
                                 (static_cast<size_t>(inst.right) << 8);
                size_t oldSlotSV = static_cast<size_t>(inst.left);
                if (oldSlotSV < remapGlobal[u].size()) {
                    size_t newSlot = remapGlobal[u][oldSlotSV];
                    if (newSlot > 0xFF) {
                        throw std::runtime_error(
                            "Linker: merged global slot index " + std::to_string(newSlot) +
                            " exceeds 8-bit STORE_VAR encoding limit (255). "
                            "Reduce the number of global variables.");
                    }
                    inst.left = static_cast<uint8_t>(newSlot);
                }
            }

            merged.push_back(inst);
            int lineNum = (i < bc.lineNumbers.size()) ? bc.lineNumbers[i] : 0;
            mergedLineNums.push_back(lineNum);
        }
    }

    // Pass 3 – Resolve cross-unit (unresolved) calls.

    for (size_t u = 0; u < units_.size(); ++u) {
        for (auto& [localIdx, funcName] : units_[u].bc.unresolvedCalls) {
            size_t absIdx = base[u] + localIdx;
            auto it = funcTable.find(funcName);
            if (it == funcTable.end()) {
                throw std::runtime_error(
                    "Linker: unresolved call to function '" + funcName + "'"
                    + (units_[u].name.empty() ? "" : " in unit '" + units_[u].name + "'"));
            }
            writeAddr(merged[absIdx], static_cast<uint16_t>(it->second));
        }
    }

    // Pass 4 – Emit the CALL main epilogue (mirrors what the compiler does in
    //          non-allowUnresolvedCalls mode).

    auto mainIt = funcTable.find("main");
    if (mainIt == funcTable.end()) {
        throw std::runtime_error("Linker: no 'main' function defined across linked units");
    }

    Instruction callMain{};
    callMain.op  = static_cast<uint32_t>(OpCode::CALL);
    callMain.dst = 0; // result discarded
    writeAddr(callMain, static_cast<uint16_t>(mainIt->second));
    merged.push_back(callMain);
    mergedLineNums.push_back(0);

    // Assemble final ByteCode.

    ByteCode out;
    out.instructions    = std::move(merged);
    out.constants       = std::move(mergedConsts);
    out.strings         = std::move(mergedStrings);
    out.lineNumbers     = std::move(mergedLineNums);
    out.globalSlotCount = mergedGlobalSlotCount;
    out.globalNamesBySlot = std::move(mergedGlobalNames);

    // Expose the merged function table for debugger / tools.
    for (auto& [name, addr] : funcTable) {
        out.functionSymbols[name] = addr;
    }

    return out;
}

bool Linker::isJumpOrCall(uint8_t op) {
    // Opcodes that carry a 16-bit instruction-address in left+right fields.
    return op == static_cast<uint8_t>(OpCode::JMP)
        || op == static_cast<uint8_t>(OpCode::JZ)
        || op == static_cast<uint8_t>(OpCode::JNZ)
        || op == static_cast<uint8_t>(OpCode::CALL)
        || op == static_cast<uint8_t>(OpCode::JAL)
        || op == static_cast<uint8_t>(OpCode::JALR);
}

void Linker::mergeConstants(
    const std::vector<Unit>& units,
    std::vector<double>& outPool,
    std::vector<std::vector<int>>& remapConst) {
    std::unordered_map<double, int> seen;
    remapConst.resize(units.size());

    for (size_t u = 0; u < units.size(); ++u) {
        const auto& pool = units[u].bc.constants;
        remapConst[u].resize(pool.size());
        for (size_t i = 0; i < pool.size(); ++i) {
            double val = pool[i];
            auto it = seen.find(val);
            if (it != seen.end()) {
                remapConst[u][i] = it->second;
            } else {
                int newIdx = static_cast<int>(outPool.size());
                outPool.push_back(val);
                seen[val] = newIdx;
                remapConst[u][i] = newIdx;
            }
        }
    }
}

void Linker::mergeStrings(
    const std::vector<Unit>& units,
    std::vector<std::string>& outPool,
    std::vector<std::vector<int>>& remapStr) {
    std::unordered_map<std::string, int> seen;
    remapStr.resize(units.size());

    for (size_t u = 0; u < units.size(); ++u) {
        const auto& pool = units[u].bc.strings;
        remapStr[u].resize(pool.size());
        for (size_t i = 0; i < pool.size(); ++i) {
            const std::string& s = pool[i];
            auto it = seen.find(s);
            if (it != seen.end()) {
                remapStr[u][i] = it->second;
            } else {
                int newIdx = static_cast<int>(outPool.size());
                outPool.push_back(s);
                seen[s] = newIdx;
                remapStr[u][i] = newIdx;
            }
        }
    }
}

void Linker::mergeGlobals(
    const std::vector<Unit>& units,
    size_t& outSlotCount,
    std::vector<std::string>& outNames,
    std::vector<std::vector<size_t>>& remapGlobal) {
        
    std::unordered_map<std::string, size_t> nameToSlot; // non-empty name -> unified slot
    remapGlobal.resize(units.size());

    outSlotCount = 0;

    for (size_t u = 0; u < units.size(); ++u) {
        const ByteCode& bc = units[u].bc;
        remapGlobal[u].resize(bc.globalSlotCount);

        for (size_t s = 0; s < bc.globalSlotCount; ++s) {
            const std::string& nm = (s < bc.globalNamesBySlot.size())
                                    ? bc.globalNamesBySlot[s]
                                    : std::string{};

            if (!nm.empty()) {
                auto it = nameToSlot.find(nm);
                if (it != nameToSlot.end()) {
                    // Reuse existing unified slot for this global name.
                    remapGlobal[u][s] = it->second;
                } else {
                    size_t newSlot = outSlotCount++;
                    nameToSlot[nm] = newSlot;
                    outNames.push_back(nm);
                    remapGlobal[u][s] = newSlot;
                }
            } else {
                // Anonymous slot - allocate fresh (no dedup).
                size_t newSlot = outSlotCount++;
                if (outNames.size() < outSlotCount) outNames.push_back("");
                remapGlobal[u][s] = newSlot;
            }
        }
    }
}