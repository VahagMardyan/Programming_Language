#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include "../VirtualMachine/vm.h"
#include "../SymbolTable/symbol_table.h"

namespace {
    std::string readAllText(const std::string& path) {
        std::ifstream file(path);
        if(!file.is_open()) {
            throw std::runtime_error("Cannot open file '" + path + "'");
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }
    
    ByteCode compileSource(const std::string& source, SymbolTable& st) {
        std::istringstream stream(source);
        Lexer lexer(stream);
        Tokenizer tokenizer(lexer);
        Parser parser(tokenizer, st);
        auto root = std::static_pointer_cast<ASTNode>(parser.parseProgram());
        if(!root) {
            throw std::runtime_error("Parsing failed");
        }
        Compiler compiler(st);
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
        SymbolTable st;

        if(modeOrFile == "compile") {
            if(argc < 3) {
                throw std::runtime_error("compile mode requires input .vhg file");
            }
            std::string inputPath = argv[2];
            if(inputPath.size() < 4 || inputPath.substr(inputPath.size() - 4) != ".vhg") {
                throw std::runtime_error("compile input must be a .vhg file");
            }

            std::string outputPath;
            if(argc >= 4) {
                outputPath = argv[3];
            } else {
                outputPath = inputPath.substr(0, inputPath.size() - 4) + ".vhb";
            }

            std::string source = readAllText(inputPath);
            ByteCode bc = compileSource(source, st);
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
        std::string source = readAllText(filename);
        VirtualMachine vm(true);
        vm.load(source, st);
        vm.run();
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}