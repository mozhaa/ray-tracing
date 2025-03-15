#pragma once

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace raytracing {

struct Pixel {
    unsigned char r, g, b;
};

Pixel aces_tonemap(glm::vec3 c);

} // namespace raytracing