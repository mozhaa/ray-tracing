#pragma once

#include <optional>

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace raytracing {

struct Object {
    glm::vec3 position;
    glm::vec4 rotation;
    glm::vec3 color;

    virtual std::optional<glm::vec3> intersect(glm::vec3 pos, glm::vec3 ray) const;
};

struct Plane : public Object {
    glm::vec3 normal;
    
    Plane(Object& obj, glm::vec3 normal);
    std::optional<glm::vec3> intersect(glm::vec3 pos, glm::vec3 ray) const override;
};

struct Ellipsoid : public Object {
    glm::vec3 radius;
    
    Ellipsoid(Object& obj, glm::vec3 radius);
    std::optional<glm::vec3> intersect(glm::vec3 pos, glm::vec3 ray) const override;
};

struct Box : public Object {
    glm::vec3 size;
    
    Box(Object& obj, glm::vec3 size);
    std::optional<glm::vec3> intersect(glm::vec3 pos, glm::vec3 ray) const override;
};

} // namespace raytracing