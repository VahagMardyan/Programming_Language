#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_set>
#include <filesystem>
#include "../Lexer/lexer.h"
#include "../Tokenizer/tokenizer.h"
#include "../Parser/parser.h"
#include "../Compiler/compiler.h"
#include "../SymbolTable/symbol_table.h"
#include "../VirtualMachine/vm.h"

namespace {
std::string readAllText(const std::filesystem::path& path) {
    std::ifstream file(path);
    if(!file.is_open()) {
        throw std::runtime_error("Cannot open file '" + path.string() + "'");
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string expandImports(
    const std::filesystem::path& path,
    std::unordered_set<std::string>& visiting,
    std::unordered_set<std::string>& seen) {
    const std::filesystem::path normalized = std::filesystem::absolute(path).lexically_normal();
    const std::string key = normalized.generic_string();
    if(visiting.count(key) > 0) {
        throw std::runtime_error("Circular import detected at '" + normalized.string() + "'");
    }
    if(seen.count(key) > 0) {
        return "";
    }

    visiting.insert(key);
    const std::string source = readAllText(normalized);
    std::istringstream sourceLines(source);
    std::ostringstream merged;
    std::string line;
    const std::regex importPattern(R"re(^\s*import\s+['"]([^'"]+)['"]\s*;?\s*$)re");

    while(std::getline(sourceLines, line)) {
        std::smatch match;
        if(std::regex_match(line, match, importPattern)) {
            const std::filesystem::path next = (normalized.parent_path() / match[1].str()).lexically_normal();
            merged << expandImports(next, visiting, seen);
            continue;
        }
        merged << line << '\n';
    }

    visiting.erase(key);
    seen.insert(key);
    return merged.str();
}

ByteCode compileWithImports(const std::string& inputPath) {
    std::unordered_set<std::string> visiting;
    std::unordered_set<std::string> seen;
    const std::string source = expandImports(inputPath, visiting, seen);
    std::istringstream stream(source);
    Lexer lexer(stream);
    Tokenizer tokenizer(lexer);
    SymbolTable symbols;
    Parser parser(tokenizer, symbols);
    auto root = std::static_pointer_cast<ASTNode>(parser.parseProgram());
    if(!root) {
        throw std::runtime_error("Parsing failed for '" + inputPath + "'");
    }
    Compiler compiler(symbols);
    return compiler.compile(root);
}
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " <filename.vhg>\n"
                  << "  " << argv[0] << " compile <input.vhg> [output.vhb]\n"
                  << "  " << argv[0] << " run <input.vhb>\n";
        return 1;
    }

    try {
        std::string modeOrFile = argv[1];

        if(modeOrFile == "compile") {
            if(argc < 3) {
                throw std::runtime_error("compile mode requires input .vhg file");
            }
            const std::string inputPath = argv[2];
            if(inputPath.size() < 4 || inputPath.substr(inputPath.size() - 4) != ".vhg") {
                throw std::runtime_error("compile input must be a .vhg file");
            }
            std::string outputPath = (argc >= 4)
                ? std::string(argv[3])
                : inputPath.substr(0, inputPath.size() - 4) + ".vhb";

            ByteCode bc = compileWithImports(inputPath);
            writeByteCodeToFile(bc, outputPath);
            std::cout << "Bytecode written to: " << outputPath << std::endl;
            return 0;
        }

        if(modeOrFile == "run") {
            if(argc < 3) {
                throw std::runtime_error("run mode requires input .vhb file");
            }
            std::string inputByteCode = argv[2];
            VirtualMachine vm(false);
            vm.loadFromFile(inputByteCode);
            vm.run();
            return 0;
        }

        // Backward-compatible mode: run source directly
        std::string filename = modeOrFile;
        if(filename.size() < 4 || filename.substr(filename.size() - 4) != ".vhg") {
            throw std::runtime_error("Expected .vhg source, or use compile/run modes");
        }
        ByteCode bc = compileWithImports(filename);
        VirtualMachine vm(false);
        vm.load(bc);
        vm.run();
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}