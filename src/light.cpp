#include "light.hpp"

namespace raytracing {

std::pair<Ray, float> LightSource::where_to_look(glm::vec3 pos) const {
    if (type == LightSourceType::Point) {
        glm::vec3 dir = position - pos;
        return {{pos, glm::normalize(dir)}, glm::length(dir)};
    } else {
        return {{pos, direction}, std::numeric_limits<float>::infinity()};
    }
}

glm::vec3 LightSource::at(glm::vec3 pos, glm::vec3 normal) const {
    glm::vec3 result = intensity;
    glm::vec3 dir = (type == LightSourceType::Direct) ? -direction : pos - position;
    result *= std::max(0.f, glm::dot(normal, -glm::normalize(dir)));
    if (type == LightSourceType::Point) {
        float R = glm::length(dir);
        result /= glm::dot(attenuation, {1, R, R * R});
    }
    return result;
}

} // namespace raytracing