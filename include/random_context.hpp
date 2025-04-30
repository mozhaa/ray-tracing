#pragma once

#include "sampling.hpp"

namespace raytracing {

struct RandomContext {
    std::minstd_rand0 rng;
    cosine_sampler S;
    std::uniform_real_distribution<float> d01;

    RandomContext(int seed = 1);
};

} // namespace raytracing
