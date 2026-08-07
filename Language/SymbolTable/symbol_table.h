#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <variant>
#include <iostream>
#include <stack>
#include <algorithm>
#include <memory>

// Forward declaration to allow Value to hold a reference-counted array of
// itself (arrays are reference types: assigning / passing an array copies
// the shared_ptr, not the underlying elements - matching array_push etc.
// mutating the "same" array seen elsewhere).
struct ArrayObj;

using Value = std::variant<std::monostate, double, std::string, std::shared_ptr<ArrayObj>>;

struct ArrayObj {
    std::vector<Value> items;
};

using ArrayRef = std::shared_ptr<ArrayObj>;

inline bool isNone(const Value& v) { return std::holds_alternative<std::monostate>(v); }
inline bool isNumber(const Value& v) { return std::holds_alternative<double>(v); }
inline bool isString(const Value& v) { return std::holds_alternative<std::string>(v); }
inline bool isArray(const Value& v) { return std::holds_alternative<ArrayRef>(v); }
inline double asNumber(const Value& v) { return std::get<double>(v); }
inline const std::string& asString(const Value& v) { return std::get<std::string>(v); }
inline const ArrayRef& asArray(const Value& v) { return std::get<ArrayRef>(v); }
inline ArrayRef makeArray(size_t size = 0) {
    auto arr = std::make_shared<ArrayObj>();
    arr->items.assign(size, Value(std::monostate{}));
    return arr;
}

inline bool isFalsy(const Value& v) {
    if(isNone(v)) return true;
    if (std::holds_alternative<double>(v)) return std::get<double>(v) == 0.0;
    if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v).empty();
    if (isArray(v)) return asArray(v)->items.empty();
    return true;
}
inline bool isTruthy(const Value& v) { return !isFalsy(v); }

inline std::string valueToString(const Value& v) {
    if(isNone(v)) return "none";
    if (std::holds_alternative<std::string>(v)) {
        return std::get<std::string>(v);
    } else if (std::holds_alternative<double>(v)) {
        double d = std::get<double>(v);
        if (d == (long long)d) return std::to_string((long long)d);
        std::string s = std::to_string(d);
        s.erase(s.find_last_not_of('0') + 1);
        if (s.back() == '.') s.pop_back();
        return s;
    } else if (isArray(v)) {
        const auto& items = asArray(v)->items;
        std::string out = "[";
        for (size_t i = 0; i < items.size(); ++i) {
            if (i) out += ", ";
            if (isString(items[i])) {
                out += "'" + valueToString(items[i]) + "'";
            } else {
                out += valueToString(items[i]);
            }
        }
        out += "]";
        return out;
    }
    return "";
}

class SymbolTable {
private:
    // Global variables
    std::unordered_map<std::string, size_t> globalAddresses;
    size_t nextGlobalAddress = 0;
    
    // Nested scope support
    struct ScopeLevel {
        std::unordered_map<std::string, int32_t> locals;
        int32_t nextOffset;
        
        ScopeLevel(int32_t startOffset = -4) : nextOffset(startOffset) {}
    };
    
    std::vector<ScopeLevel> scopeStack;  // Stack of nested scopes
    bool inFunctionScope = false;        // Are we inside a function definition?

    // Outer scope stacks while parsing nested function bodies (LIFO)
    std::vector<std::vector<ScopeLevel>> outerScopeStackStack;

    // Max stack slots needed for top-level (script) locals; updated in endProgramParse
    int programFrameSlotCount_ = 1;

public:
    bool hasGlobal(const std::string& name) const {
        return globalAddresses.find(name) != globalAddresses.end();
    }

    size_t getGlobalSlotCount() const { return nextGlobalAddress; }

    const std::unordered_map<std::string, size_t>& getGlobalAddressMap() const {
        return globalAddresses;
    }

    bool tryGetGlobalAddress(const std::string& name, size_t& addr) const {
        auto it = globalAddresses.find(name);
        if (it == globalAddresses.end()) return false;
        addr = it->second;
        return true;
    }

    size_t getGlobalAddress(const std::string& name) {
        auto it = globalAddresses.find(name);
        if (it != globalAddresses.end()) return it->second;
        size_t addr = nextGlobalAddress++;
        globalAddresses[name] = addr;
        return addr;
    }

    // Get local offset - searches from innermost to outermost scope
    bool tryGetLocalOffset(const std::string& name, int32_t& offset) const {
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
            auto found = it->locals.find(name);
            if (found != it->locals.end()) {
                offset = found->second;
                return true;
            }
        }
        return false;
    }

    int32_t getLocalOffset(const std::string& name) {
        if (scopeStack.empty()) {
            throw std::runtime_error("Cannot allocate local variable outside of any scope");
        }
        
        // Search from innermost scope outward
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
            auto found = it->locals.find(name);
            if (found != it->locals.end()) {
                return found->second;
            }
        }
        
        // Not found in any scope - allocate in current (innermost) scope
        ScopeLevel& current = scopeStack.back();
        int32_t off = current.nextOffset;
        current.nextOffset -= 4;
        current.locals[name] = off;
        return off;
    }

    // True if name is bound in the innermost scope only (for explicit "local" redeclare rules).
    bool hasLocalInInnermostScope(const std::string& name) const {
        if (scopeStack.empty()) return false;
        const auto& inner = scopeStack.back().locals;
        return inner.find(name) != inner.end();
    }

    // Look up name in enclosing function scope snapshots (nested functions). hopsOut: 1 = immediate outer, ...
    bool tryGetEnclosingLocalOffset(const std::string& name, int32_t& offset, int& hopsOut) const {
        int hop = 1;
        for (auto stackIt = outerScopeStackStack.rbegin(); stackIt != outerScopeStackStack.rend(); ++stackIt, ++hop) {
            const auto& saved = *stackIt;
            for (auto levelIt = saved.rbegin(); levelIt != saved.rend(); ++levelIt) {
                auto found = levelIt->locals.find(name);
                if (found != levelIt->locals.end()) {
                    offset = found->second;
                    hopsOut = hop;
                    return true;
                }
            }
        }
        return false;
    }

    bool tryResolveLocal(const std::string& name, int32_t& offset, int& outerHopsOut) const {
        outerHopsOut = 0;
        if (tryGetLocalOffset(name, offset)) return true;
        return tryGetEnclosingLocalOffset(name, offset, outerHopsOut);
    }

    // Enter a new function scope (saves outer scopes, starts fresh stack for the body)
    void enterFunctionScope() {
        inFunctionScope = true;
        outerScopeStackStack.push_back(std::move(scopeStack));
        scopeStack.clear();
        scopeStack.push_back(ScopeLevel(-4));  // First scope in function
    }

    // Exit function scope (restore outer / program scopes)
    void exitFunctionScope() {
        inFunctionScope = false;
        scopeStack.clear();
        if (!outerScopeStackStack.empty()) {
            scopeStack = std::move(outerScopeStackStack.back());
            outerScopeStackStack.pop_back();
        }
    }

    // Enter a nested block scope (for if/while/for/{})
    void enterBlockScope() {
        int32_t startOffset = -4;
        if (!scopeStack.empty()) {
            startOffset = scopeStack.back().nextOffset;
        }
        scopeStack.push_back(ScopeLevel(startOffset));
    }

    // Exit a nested block scope
    void exitBlockScope() {
        if (scopeStack.size() > 1) {
            // Keep track of how much stack space was used
            int32_t usedOffset = scopeStack.back().nextOffset;
            scopeStack.pop_back();
            
            // Update parent scope's offset to account for nested scope's usage
            if (!scopeStack.empty()) {
                if (usedOffset < scopeStack.back().nextOffset) {
                    scopeStack.back().nextOffset = usedOffset;
                }
            }
        }
    }

    // Lowest nextOffset across active scopes (most negative = deepest stack use).
    int32_t getMaxLocalOffset() const {
        if (scopeStack.empty()) return -4;
        
        int32_t maxOffset = -4;
        for (const auto& scope : scopeStack) {
            if (scope.nextOffset < maxOffset) {
                maxOffset = scope.nextOffset;
            }
        }
        return maxOffset;
    }

    // Slots needed for the current function frame (params + all locals), based on
    // stack offsets. Prefer this over counting scope.locals entries because inner
    // block scopes are popped after parsing while AST offsets remain valid.
    int getLocalSlotCountForFrame() const {
        if (scopeStack.empty()) return 1;
        int32_t minNext = getMaxLocalOffset();
        int n = (-minNext) / 4 - 1;
        return std::max(1, n);
    }

    bool isInsideFunction() const {
        return inFunctionScope;
    }

    bool hasActiveScope() const {
        return !scopeStack.empty();
    }

    // Program (script) parsing: one root scope for top-level locals and blocks
    void beginProgramParse() {
        programFrameSlotCount_ = 1;
        outerScopeStackStack.clear();
        scopeStack.clear();
        scopeStack.push_back(ScopeLevel(-4));
        inFunctionScope = false;
    }

    void endProgramParse() {
        if (!scopeStack.empty()) {
            programFrameSlotCount_ = std::max(programFrameSlotCount_, getLocalSlotCountForFrame());
        }
        scopeStack.clear();
    }

    int getProgramFrameSlotCount() const { return programFrameSlotCount_; }

    // Get current scope depth (for debugging)
    size_t getScopeDepth() const {
        return scopeStack.size();
    }
};