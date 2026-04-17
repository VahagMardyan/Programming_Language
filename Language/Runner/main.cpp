#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include "../VirtualMachine/vm.h"
#include "../SymbolTable/symbol_table.h"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.vhg>" << std::endl;
        return 1;
    }
    std::string filename = argv[1];
    
    if(filename.length() < 5 || filename.substr(filename.length() - 4) != ".vhg") {
        std::cerr << "Error: Only '.vhg' files are supported!" << std::endl;
        return 1;
    }

    std::ifstream file(filename);
    if(!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename <<"'" << std::endl;
        return 1;
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    SymbolTable st;
    VirtualMachine vm(true);
    try {
        vm.load(ss.str(), st);
        vm.run();
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}