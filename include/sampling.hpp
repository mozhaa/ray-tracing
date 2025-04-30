#pragma once

#include <random>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace raytracing {

struct sampler {
    std::minstd_rand0& rng;

    sampler(std::minstd_rand0& rng);
    virtual std::pair<glm::vec3, float> sample(const glm::vec3& normal) = 0;
};

struct true_uniform_sampler : public sampler {
    true_uniform_sampler(std::minstd_rand0& rng);
    std::pair<glm::vec3, float> sample(const glm::vec3& normal) override;
};

struct uniform_sampler : public sampler {
    uniform_sampler(std::minstd_rand0& rng);
    std::pair<glm::vec3, float> sample(const glm::vec3& normal) override;
};

struct cosine_sampler : public sampler {
    std::uniform_real_distribution<float> d;
    cosine_sampler(std::minstd_rand0& rng);
    std::pair<glm::vec3, float> sample(const glm::vec3& normal) override;
};

struct power_cosine_sampler : public sampler {
    float alpha;

    power_cosine_sampler(std::minstd_rand0& rng, float alpha);
    std::pair<glm::vec3, float> sample(const glm::vec3& normal) override;
};

} // namespace raytracing