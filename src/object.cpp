#include "object.hpp"

namespace raytracing {

std::optional<glm::vec3> Object::intersect(glm::vec3 pos, glm::vec3 ray) const { return {}; }

Plane::Plane(Object &obj, glm::vec3 normal) : Object(obj), normal(normal) {}

Ellipsoid::Ellipsoid(Object &obj, glm::vec3 normal) : Object(obj), radius(radius) {}

Box::Box(Object &obj, glm::vec3 normal) : Object(obj), size(size) {}

std::optional<glm::vec3> Plane::intersect(glm::vec3 pos, glm::vec3 ray) const {}

std::optional<glm::vec3> Ellipsoid::intersect(glm::vec3 pos, glm::vec3 ray) const {}

std::optional<glm::vec3> Box::intersect(glm::vec3 pos, glm::vec3 ray) const {}

} // namespace raytracing