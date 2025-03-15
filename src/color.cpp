#include "color.hpp"

namespace raytracing {

unsigned char to_pixel(float x) { return static_cast<unsigned char>(x * 255.f); }

Pixel aces_tonemap(glm::vec3 x) {
    const float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    x = (x * (a * x + b)) / (x * (c * x + d) + e);
    x = glm::clamp(x, {0.f}, {1.f});
    return {to_pixel(x.x), to_pixel(x.y), to_pixel(x.z)};
}

} // namespace raytracing
