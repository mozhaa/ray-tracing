#include "scene.hpp"

#include <atomic>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>

#include "color.hpp"
#include "image.hpp"
#include "sampling.hpp"
#include "screen_splitter.hpp"

namespace raytracing {

Scene::Scene(std::string fp) : camera(), objects() {
    std::ifstream file(fp);
    if (file.fail()) {
        throw std::runtime_error("input file does not exist");
    }

    std::string line;
    Object *object = nullptr;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        if (command == "DIMENSIONS") {
            iss >> camera.width >> camera.height;
        } else if (command == "BG_COLOR") {
            iss >> bg_color.x >> bg_color.y >> bg_color.z;
        } else if (command == "SAMPLES") {
            iss >> n_samples;
        } else if (command == "NEW_PRIMITIVE") {
            objects.emplace_back();
            object = &objects[objects.size() - 1];
        } else if (command == "PLANE") {
            object->shape = Shape::Plane;
            iss >> object->plane_normal.x >> object->plane_normal.y >> object->plane_normal.z;
            object->plane_normal = glm::normalize(object->plane_normal);
            planes.push_back(*object);
            objects.pop_back();
            object = &planes[planes.size() - 1];
        } else if (command == "ELLIPSOID") {
            object->shape = Shape::Ellipsoid;
            iss >> object->ellipsoid_radius.x >> object->ellipsoid_radius.y >> object->ellipsoid_radius.z;
        } else if (command == "BOX") {
            object->shape = Shape::Box;
            iss >> object->box_size.x >> object->box_size.y >> object->box_size.z;
        } else if (command == "TRIANGLE") {
            object->shape = Shape::Triangle;
            iss >> object->tri_A.x >> object->tri_A.y >> object->tri_A.z;
            iss >> object->tri_B.x >> object->tri_B.y >> object->tri_B.z;
            iss >> object->tri_C.x >> object->tri_C.y >> object->tri_C.z;
        } else if (command == "COLOR") {
            iss >> object->color.x >> object->color.y >> object->color.z;
        } else if (command == "POSITION") {
            iss >> object->position.x >> object->position.y >> object->position.z;
        } else if (command == "ROTATION") {
            iss >> object->rotation.x >> object->rotation.y >> object->rotation.z >> object->rotation.w;
            object->rotation = glm::normalize(object->rotation);
            object->inv_rotation = glm::inverse(object->rotation);
        } else if (command == "CAMERA_POSITION") {
            iss >> camera.position.x >> camera.position.y >> camera.position.z;
        } else if (command == "CAMERA_RIGHT") {
            iss >> camera.right.x >> camera.right.y >> camera.right.z;
        } else if (command == "CAMERA_UP") {
            iss >> camera.up.x >> camera.up.y >> camera.up.z;
        } else if (command == "CAMERA_FORWARD") {
            iss >> camera.forward.x >> camera.forward.y >> camera.forward.z;
        } else if (command == "CAMERA_FOV_X") {
            iss >> camera.fov_x;
        } else if (command == "RAY_DEPTH") {
            iss >> ray_depth;
        } else if (command == "EMISSION") {
            iss >> object->emission.x >> object->emission.y >> object->emission.z;
        } else if (command == "IOR") {
            object->material = Material::Dielectric;
            iss >> object->dielectric_ior;
        } else if (command == "DIELECTRIC") {
            object->material = Material::Dielectric;
        } else if (command == "METALLIC") {
            object->material = Material::Metallic;
        } else if (command != "") {
            std::cout << "WARNING: Unknown command: " << command << std::endl;
            continue;
        }
    }

    for (auto& obj : objects) {
        obj.center = obj.get_center();
    }

    auto begin = std::chrono::steady_clock::now();
    
    bvh.build(objects);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<float> delta = end - begin;
    std::cerr << "BVH build in " << delta.count() << "[s]" << std::endl;
}

static void show_progress(float percentage) { std::cerr << "\r" << std::round(percentage * 100) << "%" << std::flush; }

void Scene::render(std::string fp, int n_threads) const {
    int total_pixels = camera.width * camera.height;
    std::vector<Pixel> image_data(total_pixels);
    std::atomic_int pixels_done = 0;
    ScreenSplitter<8> splitter(camera.width, camera.height);

    auto job = [&]() {
        RandomContext ctx;
        std::uniform_real_distribution<float> d(0.f, 1.f);
        while (true) {
            auto [x, y, w, h] = splitter.get();
            if (x == -1) {
                break;
            }
            for (int i = x; i < w; ++i) {
                for (int j = y; j < h; ++j) {
                    glm::vec3 result_color(0.f);
                    for (int s = 0; s < n_samples; ++s) {
                        auto ray = camera.get_ray(i + d(ctx.rng), j + d(ctx.rng));
                        auto color = get_color(ray, ray_depth, ctx);
                        result_color += color;
                    }
    
                    image_data[i + j * camera.width] = aces_tonemap(result_color / static_cast<float>(n_samples));
                    ++pixels_done;    
                }
            }
        }
    };

    std::vector<std::thread> work_threads;
    work_threads.reserve(n_threads);

    for (int i = 0; i < n_threads; ++i) {
        work_threads.emplace_back(std::thread(job));
    }

    while (pixels_done != total_pixels) {
        show_progress(static_cast<float>(pixels_done) / total_pixels);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    for (int i = 0; i < n_threads; ++i) {
        work_threads[i].join();
    }

    show_progress(1.f);
    std::cout << std::endl;

    save_ppm(reinterpret_cast<const char *>(image_data.data()), camera.width, camera.height, fp.c_str());
}

std::pair<OptInsc, const Object *> Scene::intersect(Ray ray, float max_distance) const {
    std::pair<OptInsc, const Object *> nearest(std::nullopt, nullptr);

    for (auto& obj : planes) {
        auto insc = obj.intersect(ray);
        if (insc && insc.value().t < max_distance) {
            nearest.second = &obj;
            max_distance = insc.value().t;
            nearest.first = insc.value();
        }
    }

    bvh.intersect(objects, ray, nearest, max_distance);

    return nearest;
}

glm::vec3 Scene::get_color(Ray ray, int depth, RandomContext& ctx) const {
    if (depth == 0)
        return {0.f, 0.f, 0.f};

    auto [insc, p_obj] = intersect(ray);
    if (p_obj == nullptr)
        return bg_color;

    switch (p_obj->material) {
    case Material::Diffuse: {
        auto [new_dir, pdf] = ctx.S.sample(insc.value().normal);
        Ray new_ray = {ray.at(insc.value().t), new_dir};
        auto new_color = get_color(new_ray.step(), depth - 1, ctx);
        return p_obj->emission + (1.f / pdf) * (p_obj->color / glm::pi<float>()) * new_color * glm::dot(new_ray.dir, insc.value().normal);
    }
    case Material::Metallic: {
        Ray new_ray = {ray.at(insc.value().t), glm::reflect(ray.dir, insc.value().normal)};
        auto new_color = get_color(new_ray.step(), depth - 1, ctx);
        return p_obj->emission + p_obj->color * new_color;
    }
    case Material::Dielectric: {
        float eta1 = insc.value().inside ? p_obj->dielectric_ior : 1.f;
        float eta2 = insc.value().inside ? 1.f : p_obj->dielectric_ior;
        float eta = eta1 / eta2;

        float cos_theta = glm::dot(-ray.dir, insc.value().normal);
        float R0 = std::pow((eta1 - eta2) / (eta1 + eta2), 2.f);
        float r = R0 + (1 - R0) * std::pow((1 - cos_theta), 5.f);

        float sin_theta2 = eta1 / eta2 * std::sqrt(1 - std::pow(cos_theta, 2));

        if ((std::abs(sin_theta2) > 1) || (r > 0.f && (r >= 1.f || std::bernoulli_distribution(r)(ctx.rng)))) {
            Ray reflected_ray = {ray.at(insc.value().t), glm::reflect(ray.dir, insc.value().normal)};
            glm::vec3 reflected_color = get_color(reflected_ray.step(), depth - 1, ctx);
            return reflected_color;
        } else {
            Ray refracted_ray = {ray.at(insc.value().t), glm::refract(ray.dir, insc.value().normal, eta)};
            glm::vec3 refracted_color = get_color(refracted_ray.step(), depth - 1, ctx);
            if (!insc.value().inside)
                refracted_color *= p_obj->color;
            return refracted_color;
        }
    }
    default:
        assert(false);
        return glm::vec3{0.f};
    }
}

} // namespace raytracing