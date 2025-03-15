#pragma once

#include <optional>

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace raytracing {

struct Ray {
    glm::vec3 pos;
    glm::vec3 dir;

    glm::vec3 at(float t) const;
    Ray step(float t = 1e-4f) const;
};

struct Intersection {
    float t;
    glm::vec3 normal;
    bool inside;

    Intersection(float t, glm::vec3 normal, bool inside = false);
};

using OptInsc = std::optional<Intersection>;

} // namespace raytracing