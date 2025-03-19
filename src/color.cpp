#include "color.hpp"

namespace raytracing {

unsigned char to_pixel(float x) { return static_cast<unsigned char>(std::round(x * 255.f)); }

static glm::vec3 gamma_correction(glm::vec3 c) {
    const double p = 0.45454545454545454545;
    return {std::pow(c.x, p), std::pow(c.y, p), std::pow(c.z, p)};
}

Pixel aces_tonemap(glm::vec3 x) {
    const float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    x = (x * (a * x + b)) / (x * (c * x + d) + e);
    x = gamma_correction(x);
    x = glm::clamp(x, {0.f}, {1.f});
    return {to_pixel(x.x), to_pixel(x.y), to_pixel(x.z)};
}

} // namespace raytracing
