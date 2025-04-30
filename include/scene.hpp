#pragma once

#include <random>
#include <string>
#include <vector>

#include "camera.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "bvh.hpp"
#include "random_context.hpp"

namespace raytracing {

struct Scene {
    Camera camera;
    std::vector<Object> objects;
    std::vector<Object> planes;
    BVH bvh;
    glm::vec3 bg_color;
    int ray_depth;
    int n_samples;

    Scene(std::string fp);
    void render(std::string fp, int n_threads) const;

private:
    std::pair<OptInsc, const Object*> intersect(Ray ray, float max_distance = std::numeric_limits<float>::infinity()) const;
    glm::vec3 get_color(Ray ray, int depth, RandomContext& ctx) const;
};

} // namespace raytracing