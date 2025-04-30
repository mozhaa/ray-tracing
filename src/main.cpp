#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "scene.hpp"

using namespace raytracing;

int main(int argc, char **argv) {
    int n_threads = std::max(1u, std::thread::hardware_concurrency());
    std::cerr << "Using " << n_threads << " threads" << std::endl;

    auto begin = std::chrono::steady_clock::now();

    Scene scene{std::string(argv[1])};
    scene.render(std::string(argv[2]), n_threads);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<float> delta = end - begin;
    std::cerr << "Time elapsed: " << delta.count() << "[s]" << std::endl;

    return 0;
}