#include "object.hpp"

#include <stdexcept>

namespace raytracing {

static float min3(float x, float y, float z) { return std::min(x, std::min(y, z)); }

static float max3(float x, float y, float z) { return std::max(x, std::max(y, z)); }

static glm::vec3 keep_max(glm::vec3 v) {
    glm::vec3 u = v;
    v = glm::abs(v);
    if (v.x > v.y) {
        if (v.x > v.z) {
            u.y = u.z = 0;
        } else {
            u.x = u.y = 0;
        }
    } else {
        if (v.y > v.z) {
            u.x = u.z = 0;
        } else {
            u.x = u.y = 0;
        }
    }
    return u;
}

Ray Object::translate(Ray r) const {
    glm::quat inv_rotation = glm::inverse(rotation);
    return {inv_rotation * (r.pos - position), glm::normalize(inv_rotation * r.dir)};
}

glm::vec3 Object::get_center() const {
    switch (shape) {
    case Shape::Box:
        return position;
    case Shape::Ellipsoid:
        return position;
    case Shape::Plane:
        throw std::runtime_error("plane has no center");
    case Shape::Triangle:
        return (tri_A + tri_B + tri_C) / 3.f;
    default:
        throw std::runtime_error("invalid object shape");
    }
}

OptInsc Object::intersect_plane(Ray r) const {
    float t = -glm::dot(r.pos, plane_normal) / glm::dot(r.dir, plane_normal);
    if (t >= 0)
        return Intersection(t, plane_normal);
    return std::nullopt;
}

OptInsc Object::intersect_ellipsoid(Ray r) const {
    float a = glm::dot(r.dir / ellipsoid_radius, r.dir / ellipsoid_radius);
    float b = 2 * glm::dot(r.pos / ellipsoid_radius, r.dir / ellipsoid_radius);
    float c = glm::dot(r.pos / ellipsoid_radius, r.pos / ellipsoid_radius) - 1;
    float d = b * b - 4 * a * c;
    if (d < 0)
        return std::nullopt;
    d = std::sqrt(d);
    float tm = (-b - d) / (2 * a);
    float tM = (-b + d) / (2 * a);
    if (tm < 0) {
        if (tM < 0)
            return std::nullopt;
        else
            return Intersection(tM, glm::normalize(r.at(tM) / (ellipsoid_radius * ellipsoid_radius)));
    } else
        return Intersection(tm, glm::normalize(r.at(tm) / (ellipsoid_radius * ellipsoid_radius)));
}

OptInsc Object::intersect_box(Ray r) const {
    glm::vec3 tm = (-box_size - r.pos) / r.dir;
    glm::vec3 tM = (box_size - r.pos) / r.dir;
    float t1 = max3(std::min(tm.x, tM.x), std::min(tm.y, tM.y), std::min(tm.z, tM.z));
    float t2 = min3(std::max(tm.x, tM.x), std::max(tm.y, tM.y), std::max(tm.z, tM.z));
    if (t1 > t2)
        return std::nullopt;
    if (t2 < 0)
        return std::nullopt;
    if (t1 < 0)
        return Intersection(t2, glm::normalize(keep_max(r.at(t2) / box_size)));
    return Intersection(t1, glm::normalize(keep_max(r.at(t1) / box_size)));
}

OptInsc Object::intersect_triangle(Ray r) const {
    glm::mat3 m(tri_B - tri_A, tri_C - tri_A, -r.dir);
    m = glm::inverse(m);
    glm::vec3 v(r.pos - tri_A);
    v = m * v;
    if (v.x < 0 || v.y < 0 || v.x + v.y > 1 || v.z < 0) {
        return std::nullopt;
    }
    Intersection res(v.z, glm::vec3(0.f));
    res.normal = glm::normalize(glm::cross(tri_B - tri_A, tri_C - tri_A));
    if (glm::dot(res.normal, r.dir) < 0) {
        res.inside = true;
        res.normal = -res.normal;
    }
    return res;
}

OptInsc Object::intersect(Ray r) const {
    r = translate(r);
    OptInsc result = std::nullopt;
    switch (shape) {
    case Shape::Plane:
        result = intersect_plane(r);
        break;
    case Shape::Ellipsoid:
        result = intersect_ellipsoid(r);
        break;
    case Shape::Box:
        result = intersect_box(r);
        break;
    case Shape::Triangle:
        result = intersect_triangle(r);
        break;
    }
    if (result.has_value()) {
        result.value().inside = glm::dot(-r.dir, result.value().normal) < 0;
        if (result.value().inside)
            result.value().normal *= -1;
        result.value().normal = glm::normalize(rotation * result.value().normal);
    }
    return result;
}

} // namespace raytracing