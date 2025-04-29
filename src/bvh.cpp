#include "bvh.hpp"

#include <numeric>
#include <stdexcept>

namespace raytracing {

void AABB::extend(glm::vec3 p) {
    min = glm::min(min, p);
    max = glm::max(max, p);
}

void AABB::extend(AABB aabb) {
    min = glm::min(min, aabb.min);
    max = glm::max(max, aabb.max);
}

void AABB::extend(const Object &obj) {
    AABB res;

    switch (obj.shape) {
    case Shape::Box: {
        res.min = -obj.box_size;
        res.max = obj.box_size;
        break;
    }
    case Shape::Ellipsoid: {
        res.min = -obj.ellipsoid_radius;
        res.max = obj.ellipsoid_radius;
        break;
    }
    case Shape::Triangle: {
        res.extend(obj.tri_A);
        res.extend(obj.tri_B);
        res.extend(obj.tri_C);
        break;
    }
    case Shape::Plane: {
        throw std::runtime_error("can't compute aabb of a plane");
        break;
    }
    }

    auto inv_rot = glm::inverse(obj.rotation);

    extend(inv_rot * (obj.position + glm::vec3(res.min.x, res.min.y, res.min.z)));
    extend(inv_rot * (obj.position + glm::vec3(res.min.x, res.min.y, res.max.z)));
    extend(inv_rot * (obj.position + glm::vec3(res.min.x, res.max.y, res.min.z)));
    extend(inv_rot * (obj.position + glm::vec3(res.min.x, res.max.y, res.max.z)));
    extend(inv_rot * (obj.position + glm::vec3(res.max.x, res.min.y, res.min.z)));
    extend(inv_rot * (obj.position + glm::vec3(res.max.x, res.min.y, res.max.z)));
    extend(inv_rot * (obj.position + glm::vec3(res.max.x, res.max.y, res.min.z)));
    extend(inv_rot * (obj.position + glm::vec3(res.max.x, res.max.y, res.max.z)));

    // extend(obj.rotation * (obj.position + glm::vec3(res.min.x, res.min.y, res.min.z)));
    // extend(obj.rotation * (obj.position + glm::vec3(res.min.x, res.min.y, res.max.z)));
    // extend(obj.rotation * (obj.position + glm::vec3(res.min.x, res.max.y, res.min.z)));
    // extend(obj.rotation * (obj.position + glm::vec3(res.min.x, res.max.y, res.max.z)));
    // extend(obj.rotation * (obj.position + glm::vec3(res.max.x, res.min.y, res.min.z)));
    // extend(obj.rotation * (obj.position + glm::vec3(res.max.x, res.min.y, res.max.z)));
    // extend(obj.rotation * (obj.position + glm::vec3(res.max.x, res.max.y, res.min.z)));
    // extend(obj.rotation * (obj.position + glm::vec3(res.max.x, res.max.y, res.max.z)));
}

float AABB::S() const {
    glm::vec3 diag = glm::max(glm::vec3(0.f), max - min);
    return diag.x * diag.y + diag.y * diag.z + diag.z * diag.x;
}

int BVH::build_node(std::vector<Object> &primitives, int first, int count) {
    if (count == 0) {
        return -1;
    }

    Node result;
    for (const auto &p : primitives) {
        result.aabb.extend(p);
    }
    result.left_child = -1;
    result.right_child = -1;
    result.first_primitive_id = first;
    result.primitive_count = count;

    if (count <= 4) {
no_split:
        nodes.push_back(result);
        return nodes.size() - 1;
    }

    std::vector<int> indices(primitives.size(), 0);
    std::iota(indices.begin(), indices.end(), 0);

    std::function<bool(const Object &, const Object &)> cmp_x = [&](const Object &x, const Object &y) { return x.center.x < y.center.x; };
    std::function<bool(const Object &, const Object &)> cmp_y = [&](const Object &x, const Object &y) { return x.center.y < y.center.y; };
    std::function<bool(const Object &, const Object &)> cmp_z = [&](const Object &x, const Object &y) { return x.center.z < y.center.z; };
    std::vector<std::function<bool(const Object &, const Object &)>*> cmps{&cmp_x, &cmp_y, &cmp_z};

    float best_score = result.aabb.S() * count;
    int best_i = -1;
    int best_axis = -1;
    std::vector<std::vector<AABB>> left_aabbs(3, std::vector<AABB>());
    std::vector<std::vector<AABB>> right_aabbs(3, std::vector<AABB>());

    for (int axis = 0; axis < 3; ++axis) {
        std::sort(&primitives[first], &primitives[first + count], *cmps[axis]);
    
        AABB aabb;
        for (int i = first; i < first + count - 1; ++i) {
            aabb.extend(primitives[indices[i]]);
            left_aabbs[axis].push_back(aabb);
        }
        aabb = {};
        for (int i = first + count - 1; i > first; --i) {
            aabb.extend(primitives[indices[i]]);
            right_aabbs[axis].push_back(aabb);
        }

        for (int i = 0; i < count - 1; ++i) {
            float score = left_aabbs[axis][i].S() * (i + 1) + right_aabbs[axis][count - i - 2].S() * (count - i - 1);
            if (score < best_score) {
                best_score = score;
                best_axis = axis;
                best_i = i;
            }
        }
    }

    if (best_i == -1) {
        goto no_split;
    }

    if (best_axis != 2) {
        std::sort(&primitives[first], &primitives[first + count], *cmps[best_axis]);    
    }

    nodes.push_back(result);
    int result_i = nodes.size() - 1;

    result.left_child = build_node(primitives, first, best_i + 1);
    result.right_child = build_node(primitives, first + best_i + 1, count - best_i - 1);

    return result_i;
}

void BVH::build(std::vector<Object> &primitives) { root = build_node(primitives, 0, primitives.size()); }

void BVH::apply(const std::vector<Object> &primitives,
                std::function<void(const Object &)> f,
                std::function<bool(const AABB &)> pred,
                int i) const {
    if (i == -1) {
        i = root;
    }

    auto &node = nodes[i];
    if (pred(node.aabb)) {
        if (node.left_child == -1 || node.right_child == -1) {
            for (int j = node.first_primitive_id; j < node.first_primitive_id + node.primitive_count; ++j) {
                f(primitives[j]);
            }
        } else {
            apply(primitives, f, pred, node.left_child);
            apply(primitives, f, pred, node.right_child);
        }
    }
}

} // namespace raytracing