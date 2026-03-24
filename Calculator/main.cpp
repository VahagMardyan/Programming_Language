#include "vm.h"
#include <chrono>

int main() {
    SymbolTable st;
    st.setVariable("x", 1.0);
    st.setVariable("y", 2.0);
    st.setVariable("z", 3.0);

    VirtualMachine vm(true);
    std::string expr = "(x*x + y*y + z*z) * (-0.5 + x*y / 100)";

    // expr = "(((x & y) + (x | z) - (y ^ z)) * 2 / (x % 10) + -z)<<1";
    // expr = "y << z"; // 2 * 2³
    // expr = "(x+5*y/z-y>>1 + 4&x --8>>(8^6)) | 14";
    vm.load(expr, st);
    double result = vm.run(st);
    std::cout << "Result: " << result << std::endl;
    
    // const int iterations = 1000000;
    // size_t x_addr = st.getAddress("x");
    // size_t y_addr = st.getAddress("y");
    // size_t z_addr = st.getAddress("z");
    // std::cout<<"\nStarting VM execution (" << iterations <<" iterations)..."<<std::endl;
    // auto start = std::chrono::high_resolution_clock::now();
    // for(int i=0; i < iterations; ++i) {
    //     double val = static_cast<double>(i);
    //     st.setValueByAddress(x_addr, val * 0.01);
    //     st.setValueByAddress(y_addr, val * 0.02);
    //     st.setValueByAddress(z_addr, val * 0.03);
    //     double result = vm.run(st);
    //     if (i % 200000 == 0) {
    //         std::cout << "Step " << i << ": x=" << val*0.01 << ", y=" << val*0.02 << ", z=" << val*0.03 
    //                   << " => Result: " << result << std::endl;
    //     }
    // }
    // auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> diff = end - start;
    // std::cout << "\nFinished "<< iterations <<" calculations in: " << diff.count() << " seconds." << std::endl;
    return 0;
}