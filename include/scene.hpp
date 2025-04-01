#pragma once

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "camera.hpp"
#include "object.hpp"
#include "ray.hpp"

namespace raytracing {

struct Scene {
    Camera camera;
    std::vector<std::shared_ptr<Object>> objects;
    glm::vec3 bg_color;
    int ray_depth;
    int n_samples;

    Scene(std::string fp);
    void render(std::string fp) const;

private:
    std::pair<OptInsc, std::shared_ptr<Object>> intersect(Ray ray, float max_distance = std::numeric_limits<float>::infinity()) const;
    glm::vec3 get_color(Ray ray, int depth, std::minstd_rand0 &rng) const;
};

} // namespace raytracing