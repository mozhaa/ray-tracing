#pragma once

#include <vector>
#include <string>
#include <memory>

#include "camera.hpp"
#include "object.hpp"
#include "ray.hpp"

namespace raytracing {

struct Scene {
    Camera camera;
    std::vector<std::shared_ptr<Object>> objects;
    glm::vec3 bg_color;

    Scene(std::string fp);
    void render(std::string fp) const;

private:
    glm::vec3 get_pixel(Ray ray) const;
};

} // namespace raytracing