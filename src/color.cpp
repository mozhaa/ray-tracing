#include "color.hpp"

namespace raytracing {

unsigned char to_pixel(float x) { return static_cast<unsigned char>(x * 255.f); }

static glm::vec3 gamma_correction(glm::vec3 c) {
    const float p = 0.454545454545f;
    return {pow(c.x, p), pow(c.y, p), pow(c.z, p)};
}

Pixel aces_tonemap(glm::vec3 x) {
    const float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    x = (x * (a * x + b)) / (x * (c * x + d) + e);
    x = gamma_correction(x);
    x = glm::clamp(x, {0.f}, {1.f});
    return {to_pixel(x.x), to_pixel(x.y), to_pixel(x.z)};
}

} // namespace raytracing
