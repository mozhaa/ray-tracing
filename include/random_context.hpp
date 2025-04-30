#pragma once

#include "sampling.hpp"

namespace raytracing {

struct RandomContext {
    std::minstd_rand0 rng;
    cosine_sampler S;

    RandomContext(int seed = 1);
};

} // namespace raytracing
