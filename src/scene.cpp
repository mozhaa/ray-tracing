#include "scene.hpp"

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include "image.hpp"

namespace raytracing {

Scene::Scene(std::string fp) : camera(), objects() {
    std::ifstream file(fp);
    std::string line;
    std::shared_ptr<Object> obj(nullptr);

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
        } else if (command == "NEW_PRIMITIVE") {
            if (obj.get() != nullptr) {
                objects.push_back(obj);
            }
            obj = std::shared_ptr<Object>(new Object());
        } else if (command == "PLANE") {
            float x, y, z;
            iss >> x >> y >> z;
            obj = std::shared_ptr<Object>(new Plane(*obj.get(), {x, y, z}));
        } else if (command == "ELLIPSOID") {
            float x, y, z;
            iss >> x >> y >> z;
            obj = std::shared_ptr<Object>(new Ellipsoid(*obj.get(), {x, y, z}));
        } else if (command == "BOX") {
            float x, y, z;
            iss >> x >> y >> z;
            obj = std::shared_ptr<Object>(new Box(*obj.get(), {x, y, z}));
        } else if (command == "COLOR") {
            float r, g, b;
            iss >> r >> g >> b;
            obj->color = {r, g, b};
        } else if (command == "POSITION") {
            float x, y, z;
            iss >> x >> y >> z;
            obj->position = {x, y, z};
        } else if (command == "ROTATION") {
            float x, y, z, w;
            iss >> x >> y >> z >> w;
            obj->rotation = {x, y, z, w};
        } else if (command == "CAMERA_POSITION") {
            float x, y, z;
            iss >> x >> y >> z;
            camera.position = {x, y, z};
        } else if (command == "CAMERA_RIGHT") {
            float x, y, z;
            iss >> x >> y >> z;
            camera.right = {x, y, z};
        } else if (command == "CAMERA_UP") {
            float x, y, z;
            iss >> x >> y >> z;
            camera.up = {x, y, z};
        } else if (command == "CAMERA_FORWARD") {
            float x, y, z;
            iss >> x >> y >> z;
            camera.forward = {x, y, z};
        } else if (command == "CAMERA_FOV_X") {
            iss >> camera.fov_x;
        } else if (command != "") {
            std::cout << "WARNING: Unknown command: " << command << std::endl;
            continue;
        }
    }

    if (obj != nullptr) {
        objects.push_back(obj);
    }
}

struct Pixel {
    unsigned char r, g, b;
};

void Scene::render(std::string fp) const {
    std::vector<Pixel> image_data(camera.width * camera.height);

    for (int i = 0; i < camera.width; ++i) {
        for (int j = 0; j < camera.height; ++j) {
            auto ray = camera.get_ray(i, j);
            glm::vec3 pixel = get_pixel(ray) * 255.f;
            image_data[i + j * camera.width] = {
                static_cast<unsigned char>(pixel.r),
                static_cast<unsigned char>(pixel.g),
                static_cast<unsigned char>(pixel.b),
            };
        }
    }

    save_ppm(reinterpret_cast<const char*>(image_data.data()), camera.width, camera.height, fp.c_str());
}

glm::vec3 Scene::get_pixel(Ray ray) const {
    float max_d = std::numeric_limits<float>::infinity();
    glm::vec3 color = bg_color;

    for (auto p_obj : objects) {
        auto point = p_obj->intersect(ray);
        if (point) {
            float d = glm::distance(point.value(), ray.pos);
            if (d < max_d) {
                color = p_obj->color;
            }
        }
    }

    return color;
}

} // namespace raytracing