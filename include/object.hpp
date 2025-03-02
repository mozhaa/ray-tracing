#pragma once

#include <optional>

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ray.hpp"

namespace raytracing {

struct Object {
    glm::vec3 position = {0.f, 0.f, 0.f};
    glm::quat rotation = {0.f, 0.f, 0.f, 1.f};
    glm::vec3 color = {1.f, 1.f, 1.f};

    virtual std::optional<glm::vec3> intersect(Ray ray) const;

protected:
    Ray translate(Ray r) const;
};

struct Plane : public Object {
    glm::vec3 normal;
    
    Plane(Object& obj, glm::vec3 normal);
    std::optional<glm::vec3> intersect(Ray ray) const override;
};

struct Ellipsoid : public Object {
    glm::vec3 radius;
    
    Ellipsoid(Object& obj, glm::vec3 radius);
    std::optional<glm::vec3> intersect(Ray ray) const override;
};

struct Box : public Object {
    glm::vec3 size;
    
    Box(Object& obj, glm::vec3 size);
    std::optional<glm::vec3> intersect(Ray ray) const override;
};

} // namespace raytracing