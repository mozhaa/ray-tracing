#include "camera.hpp"

#include <iostream>

namespace raytracing {

Ray Camera::get_ray(float x, float y) const {
    float aspect_ratio = (float)width / height;
    float tan_fov_x = tan(fov_x / 2);
    float tan_fov_y = tan_fov_x / aspect_ratio;
    float X = (2.f * x / width - 1) * tan_fov_x;
    float Y = -(2.f * y / height - 1) * tan_fov_y;

    return {position, glm::normalize(X * right + Y * up + forward)};
}

} // namespace raytracing