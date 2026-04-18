#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <variant>
#include <iostream>

using Value = std::variant<double, std::string>;

inline bool isNumber(const Value& v) { return std::holds_alternative<double>(v); }
inline bool isString(const Value& v) { return std::holds_alternative<std::string>(v); }
inline double asNumber(const Value& v) { return std::get<double>(v); }
inline const std::string& asString(const Value& v) { return std::get<std::string>(v); }

inline bool isFalsy(const Value& v) {
    if (std::holds_alternative<double>(v)) return std::get<double>(v) == 0.0;
    if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v).empty();
    return true;
}
inline bool isTruthy(const Value& v) { return !isFalsy(v); }

inline std::string valueToString(const Value& v) {
    if (std::holds_alternative<std::string>(v)) {
        return std::get<std::string>(v);
    } else if (std::holds_alternative<double>(v)) {
        double d = std::get<double>(v);
        if (d == (long long)d) return std::to_string((long long)d);
        std::string s = std::to_string(d);
        s.erase(s.find_last_not_of('0') + 1);
        if (s.back() == '.') s.pop_back();
        return s;
    }
    return "";
}

class SymbolTable {
private:
    std::unordered_map<std::string, size_t> globalAddresses;
    std::unordered_map<std::string, int32_t> localOffsets;
    bool inFunctionScope = false;
    int32_t nextLocalOffset = -4;
    size_t nextGlobalAddress = 0;

public:
    size_t getGlobalAddress(const std::string& name) {
        auto it = globalAddresses.find(name);
        if (it != globalAddresses.end()) return it->second;
        size_t addr = nextGlobalAddress++;
        globalAddresses[name] = addr;
        return addr;
    }

    int32_t getLocalOffset(const std::string& name) {
        auto it = localOffsets.find(name);
        if (it != localOffsets.end()) return it->second;
        int32_t off = nextLocalOffset;
        nextLocalOffset -= 4;
        localOffsets[name] = off;
        return off;
    }

    bool isLocal(const std::string& name) const {
        return localOffsets.find(name) != localOffsets.end();
    }

    void enterFunctionScope() {
        inFunctionScope = true;
        localOffsets.clear();
        nextLocalOffset = -4;
    }

    void exitFunctionScope() {
        inFunctionScope = false;
        localOffsets.clear();
        nextLocalOffset = -4;
    }

    int getLocalCount() const {
        return (int)localOffsets.size();
    }

    size_t getAddress(const std::string& name) {
        return getGlobalAddress(name);
    }

    bool isInsideFunction() const {
        return inFunctionScope;
    }
};