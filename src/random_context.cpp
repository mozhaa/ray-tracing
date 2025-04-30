#include "random_context.hpp"

namespace raytracing {

RandomContext::RandomContext(int seed = 1) : rng(seed), S(rng) {}

} // namespace raytracing
