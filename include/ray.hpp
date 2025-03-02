#pragma once

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace raytracing {

struct Ray {
    glm::vec3 pos;
    glm::vec3 dir;

    glm::vec3 at(float t) const;
};

} // namespace raytracing