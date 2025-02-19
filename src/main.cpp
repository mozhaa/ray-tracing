#include <string>

#include "scene.hpp"

using namespace raytracing;

int main(int argc, char **argv) {
    Scene scene{std::string(argv[1])};
    scene.render(std::string(argv[2]));
    return 0;
}