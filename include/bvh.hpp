#pragma once

#include <functional>
#include <limits>
#include <memory>
#include <vector>


#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "object.hpp"

namespace raytracing {

struct AABB {
#define inf std::numeric_limits<float>::infinity()
    glm::vec3 min = {inf, inf, inf};
    glm::vec3 max = {-inf, -inf, -inf};

    void extend(const glm::vec3& p);
    void extend(const AABB& aabb);
    void extend(const Object &obj);
    float S() const;
};

struct Node {
    AABB aabb;
    int left_child = -1;
    int right_child = -1;
    int first_primitive_id;
    int primitive_count;
    int split_axis = -1;
};

struct BVH {
    std::vector<Node> nodes;
    int root;

    int build_node(std::vector<Object> &primitives, int first, int count);
    void build(std::vector<Object> &primitives);
    void intersect(const std::vector<Object> &primitives,
               const Ray& r,
               std::pair<OptInsc, const Object *>& nearest,
               float& max_distance,
               int i = -2) const;
};

} // namespace raytracing