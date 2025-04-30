#include "bvh.hpp"

#include <numeric>
#include <stdexcept>

namespace raytracing {

void AABB::extend(const glm::vec3& p) {
    min = glm::min(min, p);
    max = glm::max(max, p);
}

void AABB::extend(const AABB& aabb) {
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

    extend(obj.position + obj.rotation * glm::vec3(res.min.x, res.min.y, res.min.z));
    extend(obj.position + obj.rotation * glm::vec3(res.min.x, res.min.y, res.max.z));
    extend(obj.position + obj.rotation * glm::vec3(res.min.x, res.max.y, res.min.z));
    extend(obj.position + obj.rotation * glm::vec3(res.min.x, res.max.y, res.max.z));
    extend(obj.position + obj.rotation * glm::vec3(res.max.x, res.min.y, res.min.z));
    extend(obj.position + obj.rotation * glm::vec3(res.max.x, res.min.y, res.max.z));
    extend(obj.position + obj.rotation * glm::vec3(res.max.x, res.max.y, res.min.z));
    extend(obj.position + obj.rotation * glm::vec3(res.max.x, res.max.y, res.max.z));
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
    result.first_primitive_id = first;
    result.primitive_count = count;

    if (count <= 4) {
    no_split:
        for (int i = first; i < first + count; ++i) {
            result.aabb.extend(primitives[i]);
        }
        result.left_child = -1;
        result.right_child = -1;
        nodes.push_back(result);
        return nodes.size() - 1;
    }

    std::function<bool(const Object &, const Object &)> cmp_x = [&](const Object &x, const Object &y) { return x.center.x < y.center.x; };
    std::function<bool(const Object &, const Object &)> cmp_y = [&](const Object &x, const Object &y) { return x.center.y < y.center.y; };
    std::function<bool(const Object &, const Object &)> cmp_z = [&](const Object &x, const Object &y) { return x.center.z < y.center.z; };
    std::vector<std::function<bool(const Object &, const Object &)> *> cmps{&cmp_x, &cmp_y, &cmp_z};

    float best_score = 0.f;
    int best_i = -1;
    int best_axis = -1;
    std::vector<std::vector<AABB>> left_aabbs(3, std::vector<AABB>());
    std::vector<std::vector<AABB>> right_aabbs(3, std::vector<AABB>());

    for (int axis = 0; axis < 3; ++axis) {
        std::sort(&primitives[first], &primitives[first + count], *cmps[axis]);

        AABB aabb;
        for (int i = first; i < first + count - 1; ++i) {
            aabb.extend(primitives[i]);
            left_aabbs[axis].push_back(aabb);
        }
        aabb = {};
        for (int i = first + count - 1; i > first; --i) {
            aabb.extend(primitives[i]);
            right_aabbs[axis].push_back(aabb);
        }
        if (axis == 0) {
            result.aabb = aabb;
            result.aabb.extend(primitives[first]);
            best_score = result.aabb.S() * count;
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

    result.split_axis = best_axis;

    nodes.push_back(result);
    int result_i = nodes.size() - 1;

    nodes[result_i].left_child = build_node(primitives, first, best_i + 1);
    nodes[result_i].right_child = build_node(primitives, first + best_i + 1, count - best_i - 1);

    return result_i;
}

void BVH::build(std::vector<Object> &primitives) { root = build_node(primitives, 0, primitives.size()); }

static float min3(float x, float y, float z) { return std::min(x, std::min(y, z)); }

static float max3(float x, float y, float z) { return std::max(x, std::max(y, z)); }

void BVH::intersect(
    const std::vector<Object> &primitives, const Ray& r, std::pair<OptInsc, const Object *> &nearest, float &max_distance, int i) const {
    if (i == -2) {
        i = root;
    }

    auto &node = nodes[i];
    glm::vec3 tm = (node.aabb.min - r.pos) / r.dir;
    glm::vec3 tM = (node.aabb.max - r.pos) / r.dir;
    float t1 = max3(std::min(tm.x, tM.x), std::min(tm.y, tM.y), std::min(tm.z, tM.z));
    float t2 = min3(std::max(tm.x, tM.x), std::max(tm.y, tM.y), std::max(tm.z, tM.z));
    if (t1 > t2 || t2 < 0) {
        return;
    }

    if (t1 >= max_distance) {
        return;
    }
    if (node.left_child == -1 || node.right_child == -1) {
        for (int j = node.first_primitive_id; j < node.first_primitive_id + node.primitive_count; ++j) {
            auto insc = primitives[j].intersect(r);
            if (insc && insc.value().t < max_distance) {
                nearest.second = &primitives[j];
                max_distance = insc.value().t;
                nearest.first = insc.value();
            }
        }
    } else {
        bool reverse_order = false;
        switch (node.split_axis) {
        case 0:
            reverse_order = r.dir.x < 0;
            break;
        case 1:
            reverse_order = r.dir.y < 0;
            break;
        case 2:
            reverse_order = r.dir.z < 0;
            break;
        default:
            throw std::runtime_error("split axis must be 0, 1 or 2");
        }
        if (reverse_order) {
            intersect(primitives, r, nearest, max_distance, node.right_child);
            intersect(primitives, r, nearest, max_distance, node.left_child);
        } else {
            intersect(primitives, r, nearest, max_distance, node.left_child);
            intersect(primitives, r, nearest, max_distance, node.right_child);
        }
    }
}

} // namespace raytracing