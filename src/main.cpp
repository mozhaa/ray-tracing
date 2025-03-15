#include <chrono>
#include <iostream>
#include <string>

#include "scene.hpp"

using namespace raytracing;

int main(int argc, char **argv) {
    auto begin = std::chrono::steady_clock::now();

    Scene scene{std::string(argv[1])};
    scene.render(std::string(argv[2]));

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<float> delta = end - begin;
    std::cout << "Time elapsed: " << delta.count() << "[s]" << std::endl;

    return 0;
}