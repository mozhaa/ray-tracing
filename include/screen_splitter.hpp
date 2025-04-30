#pragma once

#include <mutex>

namespace raytracing {

template <int tile_size = 8> struct ScreenSplitter {
    ScreenSplitter(int width, int height) : width(width), height(height), x(0), y(0), done(false) {}

    std::tuple<int, int, int, int> get() {
        if (done) {
            return {-1, -1, -1, -1};
        }
        lock.lock();
        std::tuple<int, int, int, int> tile = {x, y, std::min(x + tile_size, width), std::min(y + tile_size, height)};
        x += tile_size;
        if (x >= width) {
            x = 0;
            y += tile_size;
            if (y >= height) {
                done = true;
            }
        }
        lock.unlock();
        return tile;
    }

private:
    std::mutex lock;
    int width, height, x, y;
    bool done;
};

} // namespace raytracing
