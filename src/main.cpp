#include <string>
#include <chrono>
#include <iostream>
#include <functional>

#include "scene.hpp"

using namespace raytracing;

float timeit(std::function<void(void)> f) {
    auto begin = std::chrono::steady_clock::now();
    f();
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<float> delta = end - begin;
    return delta.count();
}

int main(int argc, char **argv) {
    auto elapsed_time = timeit([&argv](){
        Scene scene{std::string(argv[1])};
        scene.render(std::string(argv[2]));
    });
    std::cout << "Time elapsed: " << elapsed_time << "[s]" << std::endl;
    return 0;
}