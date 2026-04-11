#pragma once

#include <vector>
#include <unordered_map>
#include <stack>
#include <stdexcept>
#include <memory>
#include <string>
#include <cctype>
#include <variant>
#include <iostream>

using Value = std::variant<double, std::string>;

inline bool isNumber(const Value& v) {
    return std::holds_alternative<double>(v);
}

inline bool isString(const Value& v) {
    return std::holds_alternative<std::string>(v);
}

inline double asNumber(const Value& v) {
    return std::get<double>(v);
}

inline const std::string& asString(const Value& v) {
    return std::get<std::string>(v);
}

inline bool isFalsy(const Value& v) {
    if(std::holds_alternative<double>(v)) {
        return std::get<double>(v) == 0.0;
    }
    if(std::holds_alternative<std::string>(v)) {
        return std::get<std::string>(v).empty();
    }
    return true;
}

inline bool isTruthy(const Value& v) {
    return !isFalsy(v);
}

class SymbolTable {
    private:
        std::unordered_map<std::string, size_t> nameToIndex;
        std::vector<Value> memory;

    public:
        size_t getAddress(const std::string&);
        void setValueByAddress(size_t, const Value&);
        Value getValueByAddress(size_t) const;
        void setVariable(const std::string&, const Value&);
        Value getValue(const std::string&) const;
};
