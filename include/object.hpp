#pragma once

#include <variant>

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "ray.hpp"

namespace raytracing {

enum Material { Diffuse, Metallic, Dielectric };

enum Shape { Plane, Ellipsoid, Box, Triangle };

struct Object {
    glm::vec3 position = {0.f, 0.f, 0.f};
    glm::vec3 center = {0.f, 0.f, 0.f};
    glm::quat rotation = {1.f, 0.f, 0.f, 0.f};
    glm::quat inv_rotation = {1.f, 0.f, 0.f, 0.f};
    glm::vec3 color = {1.f, 1.f, 1.f};
    glm::vec3 emission = {0.f, 0.f, 0.f};
    Material material = Material::Diffuse;
    Shape shape = Shape::Plane;

    glm::vec3 plane_normal = {0.f, 0.f, 1.f};
    glm::vec3 ellipsoid_radius = {1.f, 1.f, 1.f};
    glm::vec3 box_size = {1.f, 1.f, 1.f};
    glm::vec3 tri_A = {0.f, 0.f, 0.f};
    glm::vec3 tri_B = {0.f, 0.f, 0.f};
    glm::vec3 tri_C = {0.f, 0.f, 0.f};
    float dielectric_ior = 1.33f;

    Ray translate(Ray r) const;
    OptInsc intersect(Ray r) const;

    glm::vec3 get_center() const;

private:
    OptInsc intersect_plane(Ray r) const;
    OptInsc intersect_ellipsoid(Ray r) const;
    OptInsc intersect_box(Ray r) const;
    OptInsc intersect_triangle(Ray r) const;
};

} // namespace raytracing