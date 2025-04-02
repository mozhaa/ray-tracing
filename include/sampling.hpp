#pragma once

#include <random>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace raytracing {

struct sampler {
    glm::vec3 normal;
    std::minstd_rand0& rng;

    sampler(glm::vec3 normal, std::minstd_rand0& rng);
    virtual std::pair<glm::vec3, float> sample() const = 0;
};

struct true_uniform_sampler : public sampler {
    true_uniform_sampler(glm::vec3 normal, std::minstd_rand0& rng);
    std::pair<glm::vec3, float> sample() const override;
};

struct uniform_sampler : public sampler {
    uniform_sampler(glm::vec3 normal, std::minstd_rand0& rng);
    std::pair<glm::vec3, float> sample() const override;
};

struct cosine_sampler : public sampler {
    cosine_sampler(glm::vec3 normal, std::minstd_rand0& rng);
    std::pair<glm::vec3, float> sample() const override;
};

struct power_cosine_sampler : public sampler {
    float alpha;

    power_cosine_sampler(glm::vec3 normal, std::minstd_rand0& rng, float alpha);
    std::pair<glm::vec3, float> sample() const override;
};

} // namespace raytracing