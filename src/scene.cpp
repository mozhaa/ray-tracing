#include "scene.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "stb_image_write.h"

namespace raytracing {

Scene::Scene(std::string fp) : camera(), objects() {
    std::ifstream file(fp);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        if (command == "DIMENSIONS") {
            iss >> camera.width >> camera.height;
        } else if (command == "BG_COLOR") {
            float r, g, b;
            iss >> r >> g >> b;
            bg_color = {r, g, b};
        } else {
            std::cout << "WARNING: Unknown command: " << command << std::endl;
            continue;
        }
    }
}

void Scene::render(std::string fp) const {
    unsigned char *data = new unsigned char[camera.width * camera.height * 3];
    stbi_write_png(fp.c_str(), camera.width, camera.height, 3, data, camera.width * 3);
}

} // namespace raytracing