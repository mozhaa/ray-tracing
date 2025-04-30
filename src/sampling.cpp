#include "sampling.hpp"

#include "glm/gtx/rotate_vector.hpp"

namespace raytracing {

sampler::sampler(std::minstd_rand0 &rng) : rng(rng) {}

static inline glm::vec3 from_spherical(float theta, float phi, glm::vec3 normal) {
    glm::vec3 v = glm::cross(normal, normal + glm::vec3(1.f, 1.f, 1.f));
    return glm::normalize(glm::rotate(glm::rotate({normal, 0.f}, theta, v), phi, normal).xyz());
}

true_uniform_sampler::true_uniform_sampler(std::minstd_rand0 &rng) : sampler(rng) {}

std::pair<glm::vec3, float> true_uniform_sampler::sample(glm::vec3 normal) const {
    std::normal_distribution<float> d(0.f, 1.f);
    glm::vec3 v = {d(rng), d(rng), d(rng)};
    v = glm::normalize(v);
    if (glm::dot(v, normal) < 0)
        return {-v, glm::one_over_two_pi<float>()};
    else
        return {v, glm::one_over_two_pi<float>()};
}

uniform_sampler::uniform_sampler(std::minstd_rand0 &rng) : sampler(rng) {}

std::pair<glm::vec3, float> uniform_sampler::sample(glm::vec3 normal) const {
    std::uniform_real_distribution<float> d(0.f, 1.f);
    float e1 = d(rng), e2 = d(rng);
    float theta = std::acos(e1);
    float phi = glm::two_pi<float>() * e2;
    return {from_spherical(theta, phi, normal), glm::one_over_two_pi<float>()};
}

cosine_sampler::cosine_sampler(std::minstd_rand0 &rng) : sampler(rng) {}

std::pair<glm::vec3, float> cosine_sampler::sample(glm::vec3 normal) const {
    std::uniform_real_distribution<float> d(0.f, 1.f);
    float e1 = d(rng), e2 = d(rng);
    float theta = std::acos(std::sqrt(e1));
    float phi = glm::two_pi<float>() * e2;
    return {from_spherical(theta, phi, normal), std::cos(theta) * glm::one_over_pi<float>()};
}

power_cosine_sampler::power_cosine_sampler(std::minstd_rand0 &rng, float alpha) : sampler(rng), alpha(alpha) {}

std::pair<glm::vec3, float> power_cosine_sampler::sample(glm::vec3 normal) const {
    std::uniform_real_distribution<float> d(0.f, 1.f);
    float e1 = d(rng), e2 = d(rng);
    float theta = std::acos(std::pow(e1, 1.f / (1.f + alpha)));
    float phi = glm::two_pi<float>() * e2;
    return {from_spherical(theta, phi, normal), std::pow(std::cos(theta), alpha) * (alpha + 1.f) * glm::one_over_two_pi<float>()};
}

} // namespace raytracing