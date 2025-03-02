#include "ray.hpp"

namespace raytracing {

glm::vec3 Ray::at(float t) const {
    return pos + dir * t;
}

} // namespace raytracing