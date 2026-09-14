#include <iostream>
#include <chrono>
#include "movegen/movegen.hpp"

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    init_all();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "init_all() took: " << duration.count() << " ms" << std::endl;



    return 0;
}