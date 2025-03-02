#include "object.hpp"

namespace raytracing {

std::optional<glm::vec3> Object::intersect(Ray ray) const { return {}; }

Ray Object::translate(Ray r) const { 
    glm::quat inv_rotation = glm::inverse(rotation);
    return {inv_rotation * (r.pos - position), glm::normalize(inv_rotation * r.dir)};
}

Plane::Plane(Object &obj, glm::vec3 normal) : Object(obj), normal(normal) {}

Ellipsoid::Ellipsoid(Object &obj, glm::vec3 radius) : Object(obj), radius(radius) {}

Box::Box(Object &obj, glm::vec3 size) : Object(obj), size(size) {}

std::optional<glm::vec3> Plane::intersect(Ray ray) const {
    ray = translate(ray);
    float t = -glm::dot(ray.pos, normal) / glm::dot(ray.dir, normal);
    if (t >= 0)
        return ray.at(t);
    return {};
}

std::optional<glm::vec3> Ellipsoid::intersect(Ray ray) const {
    ray = translate(ray);
    float a = glm::dot(ray.dir / radius, ray.dir / radius);
    float b = 2 * glm::dot(ray.pos / radius, ray.dir / radius);
    float c = glm::dot(ray.pos / radius, ray.pos / radius) - 1;
    float d = b * b - 4 * a * c;
    if (d < 0)
        return {};
    d = sqrt(d);
    float tm = (-b - d) / (2 * a);
    float tM = (-b + d) / (2 * a);
    if (tm < 0) {
        if (tM < 0)
            return {};
        else
            return ray.at(tM);
    } else
        return ray.at(tm);
}

static float min3(float x, float y, float z) {
    return std::min(x, std::min(y, z));
}

static float max3(float x, float y, float z) {
    return std::max(x, std::max(y, z));
}

std::optional<glm::vec3> Box::intersect(Ray ray) const {
    ray = translate(ray);
    glm::vec3 tm = (-size - ray.pos) / ray.dir;
    glm::vec3 tM = (size - ray.pos) / ray.dir;
    float t1 = max3(
        std::min(tm.x, tM.x),
        std::min(tm.y, tM.y),
        std::min(tm.z, tM.z)
    );
    float t2 = min3(
        std::max(tm.x, tM.x),
        std::max(tm.y, tM.y),
        std::max(tm.z, tM.z)
    );
    if (t1 > t2)
        return {};
    if (t2 < 0)
        return {};
    if (t1 < 0)
        return ray.at(t2);
    return ray.at(t1);
}

} // namespace raytracing