#include "ray.hpp"

namespace raytracing {

glm::vec3 Ray::at(float t) const { return pos + dir * t; }

Ray Ray::step(float t) const { return {pos + dir * t, dir}; }

Intersection::Intersection(float t, glm::vec3 normal, bool inside) : t(t), normal(glm::normalize(normal)), inside(inside) {}

} // namespace raytracing