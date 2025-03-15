#pragma once

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "ray.hpp"

#include <vector>

namespace raytracing {

enum LightSourceType {
    Direct,
    Point
};

struct LightSource {
    glm::vec3 intensity = {1.0, 1.0, 1.0};
    glm::vec3 attenuation = {1.0, 1.0, 1.0};

    LightSourceType type = LightSourceType::Direct;
    glm::vec3 position = {0.0, 0.0, 0.0};
    glm::vec3 direction = {0.0, 0.0, -1.0};

    std::pair<Ray, float> where_to_look(glm::vec3 pos) const;
    glm::vec3 at(glm::vec3 pos, glm::vec3 normal) const;
};

struct AmbientLighting {
    glm::vec3 color = {.2f, .2f, .2f};
};

} // namespace raytracing