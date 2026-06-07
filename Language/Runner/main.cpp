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
#include "../Linker/linker.h"

namespace {

std::string readAllText(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file '" + path.string() + "'");
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string expandImports(
    const std::filesystem::path& path,
    std::unordered_set<std::string>& visiting,
    std::unordered_set<std::string>& seen)
{
    const std::filesystem::path normalized = std::filesystem::absolute(path).lexically_normal();
    const std::string key = normalized.generic_string();
    if (visiting.count(key))
        throw std::runtime_error("Circular import detected at '" + normalized.string() + "'");
    if (seen.count(key))
        return "";

    visiting.insert(key);
    const std::string source = readAllText(normalized);
    std::istringstream sourceLines(source);
    std::ostringstream merged;
    std::string line;
    const std::regex importPattern(R"re(^\s*import\s+['"]([^'"]+)['"]\s*;?\s*$)re");

    while (std::getline(sourceLines, line)) {
        std::smatch match;
        if (std::regex_match(line, match, importPattern)) {
            const std::filesystem::path next =
                (normalized.parent_path() / match[1].str()).lexically_normal();
            merged << expandImports(next, visiting, seen);
            continue;
        }
        merged << line << '\n';
    }

    visiting.erase(key);
    seen.insert(key);
    return merged.str();
}

// Compile a single .vhg source (with import expansion) into ByteCode.
// allowUnresolved=true is used when producing object units for the linker.

ByteCode compileSource(
    const std::string& inputPath,
    bool allowUnresolved = false,
    bool emitMainFramePrologue = true) {
    std::unordered_set<std::string> visiting, seen;
    const std::string source = expandImports(inputPath, visiting, seen);
    std::istringstream stream(source);
    Lexer lexer(stream);
    Tokenizer tokenizer(lexer);
    SymbolTable symbols;
    Parser parser(tokenizer, symbols);
    auto root = std::static_pointer_cast<ASTNode>(parser.parseProgram(!allowUnresolved));
    if (!root)
        throw std::runtime_error("Parsing failed for '" + inputPath + "'");
    Compiler compiler(symbols);
    return compiler.compile(root, /*allowUnresolvedCalls=*/allowUnresolved,
                            /*emitMainFramePrologue=*/emitMainFramePrologue);
}

// Legacy name kept for backward compat
ByteCode compileWithImports(const std::string& inputPath) {
    return compileSource(inputPath, /*allowUnresolved=*/false);
}

void printUsage(const char* prog) {
    std::cerr
        << "Usage:\n"
        << "  " << prog << " <file.vhg>                    Build and run source directly\n"
        << "  " << prog << " compile <in.vhg> [out.vhb]    Compile to bytecode object\n"
        << "  " << prog << " compile-obj <in.vhg> [out.vhb] Compile to linkable object\n"
        << "  " << prog << " link <a.vhb> [b.vhb ...] -o <out.vhb>  Link objects\n"
        << "  " << prog << " run <in.vhb> [--debug]         Run bytecode\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    try {
        std::string mode = argv[1];

        // compile — full compile (must have main, not linkable)
        if (mode == "compile") {
            if (argc < 3) throw std::runtime_error("compile: requires input .vhg file");
            const std::string inputPath = argv[2];
            if (inputPath.size() < 4 || inputPath.substr(inputPath.size() - 4) != ".vhg")
                throw std::runtime_error("compile: input must be a .vhg file");

            std::string outputPath = (argc >= 4)
                ? std::string(argv[3])
                : inputPath.substr(0, inputPath.size() - 4) + ".vhb";

            ByteCode bc = compileWithImports(inputPath);
            writeByteCodeToFile(bc, outputPath);
            std::cout << "Bytecode written to: " << outputPath << std::endl;
            return 0;
        }

        // compile-obj — compile to a linkable object unit (.vhb)
        //               cross-unit calls are allowed to remain unresolved
        if (mode == "compile-obj") {
            if (argc < 3) throw std::runtime_error("compile-obj: requires input .vhg file");
            const std::string inputPath = argv[2];
            if (inputPath.size() < 4 || inputPath.substr(inputPath.size() - 4) != ".vhg")
                throw std::runtime_error("compile-obj: input must be a .vhg file");

            std::string outputPath = (argc >= 4)
                ? std::string(argv[3])
                : inputPath.substr(0, inputPath.size() - 4) + ".vhb";

            // Object units omit the program entry prologue; the linker emits
            // CALL main (and a single prologue) in the final executable.
            ByteCode bc = compileSource(inputPath, /*allowUnresolved=*/true,
                                        /*emitMainFramePrologue=*/false);
            writeByteCodeToFile(bc, outputPath);
            std::cout << "Object unit written to: " << outputPath << std::endl;
            return 0;
        }

        // link — merge multiple .vhb object units into one executable .vhb
        //   vhg link a.vhb b.vhb lib.vhb -o program.vhb
        if (mode == "link") {
            // Collect inputs and -o output
            std::vector<std::string> inputs;
            std::string outputPath;
            bool nextIsOutput = false;

            for (int i = 2; i < argc; ++i) {
                std::string arg = argv[i];
                if (nextIsOutput) {
                    outputPath = arg;
                    nextIsOutput = false;
                } else if (arg == "-o") {
                    nextIsOutput = true;
                } else {
                    inputs.push_back(arg);
                }
            }

            if (inputs.empty())
                throw std::runtime_error("link: no input files");
            if (outputPath.empty())
                throw std::runtime_error("link: missing -o <output.vhb>");

            Linker linker;
            for (const auto& inp : inputs) {
                ByteCode unit = readByteCodeFromFile(inp);
                linker.addUnit(std::move(unit), inp);
                std::cout << "  loaded: " << inp << "\n";
            }

            ByteCode linked = linker.link();
            writeByteCodeToFile(linked, outputPath);
            std::cout << "Linked bytecode written to: " << outputPath << std::endl;
            return 0;
        }

        // run — execute a .vhb bytecode file
        if (mode == "run") {
            if (argc < 3) throw std::runtime_error("run: requires input .vhb file");
            bool debugFlag = false;
            std::string inputByteCode;
            for (int i = 2; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--debug") debugFlag = true;
                else inputByteCode = arg;
            }
            VirtualMachine vm(debugFlag);
            vm.loadFromFile(inputByteCode);
            vm.run();
            return 0;
        }
        // Backward-compatible mode: run .vhg source directly
        std::string filename = mode;
        if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".vhg")
            throw std::runtime_error("Expected .vhg source, or use compile / compile-obj / link / run");

        ByteCode bc = compileWithImports(filename);
        VirtualMachine vm(false);
        vm.load(bc);
        vm.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}