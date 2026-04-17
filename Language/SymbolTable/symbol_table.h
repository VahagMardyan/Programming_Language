#pragma once
#include <vector>
#include <unordered_map>
#include <stack>
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
    struct Scope {
        std::unordered_map<std::string, size_t> nameToOffset;
        size_t nextOffset;
    };
    std::vector<Scope> scopes;
public:
    SymbolTable() { enterScope(); }
    void enterScope() { scopes.push_back({}); }
    void exitScope() { if (scopes.size() > 1) scopes.pop_back(); }
    
    size_t getOffset(const std::string& name) {
        for (int i = (int)scopes.size()-1; i >= 0; --i) {
            auto it = scopes[i].nameToOffset.find(name);
            if (it != scopes[i].nameToOffset.end())
                return it->second;
        }
        size_t off = scopes.back().nextOffset++;
        scopes.back().nameToOffset[name] = off;
        return off;
    }
    
    size_t getCurrentScopeSize() const {
        return scopes.back().nextOffset;
    }
};