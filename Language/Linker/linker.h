#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "../Compiler/compiler.h"

class Linker {
public:
    struct Unit {
        ByteCode bc;
        std::string name; // file name / label used in error messages
    };

    void addUnit(ByteCode bc, const std::string& name = "");
    ByteCode link();

private:
    std::vector<Unit> units_;

    /** Rewrite a single 16-bit jump/call address embedded in left+right fields. */
    static uint16_t readAddr(const Instruction& i)  { return getAddress(i); }
    static void     writeAddr(Instruction& i, uint16_t a) { setAddress(i, a); }

    /** True for opcodes that carry an instruction-pointer address. */
    static bool isJumpOrCall(uint8_t op);

    /**
     * Build the unified constant pool from all units.
     * Returns per-unit remapping tables: remapConst[u][oldIdx] = newIdx.
     */
    void mergeConstants(
        const std::vector<Unit>& units,
        std::vector<double>& outPool,
        std::vector<std::vector<int>>& remapConst);

    /**
     * Build the unified string pool from all units.
     * Returns per-unit remapping tables: remapStr[u][oldIdx] = newIdx.
     */
    void mergeStrings(
        const std::vector<Unit>& units,
        std::vector<std::string>& outPool,
        std::vector<std::vector<int>>& remapStr);

    /**
     * Build the unified global-variable table.
     * Returns per-unit remapping tables: remapGlobal[u][oldSlot] = newSlot.
    */
    void mergeGlobals(
        const std::vector<Unit>& units,
        size_t& outSlotCount,
        std::vector<std::string>& outNames,
        std::vector<std::vector<size_t>>& remapGlobal);
};