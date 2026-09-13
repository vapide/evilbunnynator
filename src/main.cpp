#include <iostream>
#include <chrono>
#include "movegen/magics.hpp"

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    Magics::init();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Magics::init() took: " << duration.count() << " ms" << std::endl;

    return 0;
}