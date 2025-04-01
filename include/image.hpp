#pragma once

#include <string>

namespace raytracing {

void save_ppm(const char *data, int width, int height, std::string fp);

} // namespace raytracing