#include "random_context.hpp"

namespace raytracing {

RandomContext::RandomContext(int seed) : rng(seed), S(rng), d01(0.f, 1.f) {}

} // namespace raytracing
