#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "vm.h"
#include "symbol_table.h"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    std::ifstream file(argv[1]);
    if(!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << argv[1] << "'" << std::endl;
        return 1;
    }
    std::ostringstream ss;
    ss << file.rdbuf();

    SymbolTable st;
    VirtualMachine vm(true);
    try {
        vm.load(ss.str(), st);
        vm.run(st);
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}