#include "camera.hpp"

#include <iostream>

namespace raytracing {

Ray Camera::get_ray(int i, int j) const {
    float aspect_ratio = (float)width / height;
    float tan_fov_x = tan(fov_x / 2);
    float tan_fov_y = tan_fov_x / aspect_ratio;
    float X = (2.f * (i + 0.5f) / width - 1) * tan_fov_x;
    float Y = -(2.f * (j + 0.5f) / height - 1) * tan_fov_y;
    
    return {position, glm::normalize(X * right + Y * up + forward)};
}

} // namespace raytracing