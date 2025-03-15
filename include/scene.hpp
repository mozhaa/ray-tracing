#pragma once

#include <memory>
#include <string>
#include <vector>

#include "camera.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "light.hpp"

namespace raytracing {

struct Scene {
    Camera camera;
    std::vector<std::shared_ptr<Object>> objects;
    std::vector<std::shared_ptr<LightSource>> light_sources;
    AmbientLighting ambient;
    glm::vec3 bg_color;
    int ray_depth;

    Scene(std::string fp);
    void render(std::string fp) const;

  private:
    std::pair<OptInsc, std::shared_ptr<Object>> intersect(Ray ray, float max_distance = std::numeric_limits<float>::infinity()) const;
    glm::vec3 get_color(Ray ray, int depth) const;
};

} // namespace raytracing