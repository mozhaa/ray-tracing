#include "image.hpp"

#include <fstream>

namespace raytracing {

void save_ppm(const char *data, int width, int height, std::string fp) {
    std::ofstream f(fp, std::ios::binary);
    f << "P6\n" << width << ' ' << height << "\n255\n";
    f.write(data, width * height * 3);
    f.close();
}

} // namespace raytracing