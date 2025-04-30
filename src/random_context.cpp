#include "random_context.hpp"

namespace raytracing {

RandomContext::RandomContext(int seed) : rng(seed), S(rng) {}

} // namespace raytracing
