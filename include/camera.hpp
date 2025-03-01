#pragma once

#include <vector>

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "ray.hpp"

namespace raytracing {

struct Camera {
    int width, height;
    glm::vec3 position, right, up, forward;
    float fov_x;

    Ray get_ray(int i, int j) const;
};

} // namespace raytracing