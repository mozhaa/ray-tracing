#pragma once

#include <vector>
#include <string>

#include "camera.hpp"
#include "object.hpp"

namespace raytracing {

struct Scene {
    Camera camera;
    std::vector<Object> objects;
    glm::vec3 bg_color;

    Scene(std::string fp);
    void render(std::string fp) const;
};

} // namespace raytracing