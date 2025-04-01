#include "scene.hpp"

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#ifdef MULTITHREADING_ENABLED
#include "thread_pool/BS_thread_pool.hpp"
const int n_threads = 8;
#endif

#include "color.hpp"
#include "image.hpp"

namespace raytracing {

Scene::Scene(std::string fp) : camera(), objects() {
    std::ifstream file(fp);
    if (file.fail()) {
        throw std::runtime_error("input file does not exist");
    }

    std::string line;

    std::shared_ptr<Object> object(nullptr);

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
            if (object.get() != nullptr)
                objects.push_back(object);
            object = std::shared_ptr<Object>(new Object());
        } else if (command == "PLANE") {
            object->shape = Shape::Plane;
            iss >> object->plane_normal.x >> object->plane_normal.y >> object->plane_normal.z;
            object->plane_normal = glm::normalize(object->plane_normal);
        } else if (command == "ELLIPSOID") {
            object->shape = Shape::Ellipsoid;
            iss >> object->ellipsoid_radius.x >> object->ellipsoid_radius.y >> object->ellipsoid_radius.z;
        } else if (command == "BOX") {
            object->shape = Shape::Box;
            iss >> object->box_size.x >> object->box_size.y >> object->box_size.z;
        } else if (command == "COLOR") {
            iss >> object->color.x >> object->color.y >> object->color.z;
        } else if (command == "POSITION") {
            iss >> object->position.x >> object->position.y >> object->position.z;
        } else if (command == "ROTATION") {
            iss >> object->rotation.x >> object->rotation.y >> object->rotation.z >> object->rotation.w;
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

    if (object != nullptr)
        objects.push_back(object);
}

void Scene::render(std::string fp) const {
    std::vector<Pixel> image_data(camera.width * camera.height);

    auto fill_area = [&](const std::size_t start, const std::size_t end) {
        std::minstd_rand0 rng;
        for (std::size_t t = start; t < end; ++t) {
            auto j = t % camera.height;
            auto i = (t - j) / camera.height;
            std::uniform_real_distribution<float> d(0.f, 1.f);
            glm::vec3 result_color(0.f);
            for (int s = 0; s < n_samples; ++s) {
                auto ray = camera.get_ray(i + d(rng), j + d(rng));
                auto color = get_color(ray, ray_depth, rng);
                result_color += color;
            }
            image_data[i + j * camera.width] = aces_tonemap(result_color / static_cast<float>(n_samples));
        }
    };

#ifdef MULTITHREADING_ENABLED
    BS::thread_pool pool;
    pool.submit_blocks(0, camera.width * camera.height, fill_area, n_threads).wait();
#else
    fill_area(0, camera.width * camera.height);
#endif

    save_ppm(reinterpret_cast<const char *>(image_data.data()), camera.width, camera.height, fp.c_str());
}

std::pair<OptInsc, std::shared_ptr<Object>> Scene::intersect(Ray ray, float max_distance) const {
    std::pair<OptInsc, std::shared_ptr<Object>> nearest(std::nullopt, nullptr);

    for (auto p_obj : objects) {
        auto insc = p_obj->intersect(ray);
        if (insc && insc.value().t < max_distance) {
            nearest.second = p_obj;
            max_distance = insc.value().t;
            nearest.first = insc.value();
        }
    }

    return nearest;
}

static glm::vec3 get_random_reflection(glm::vec3 normal, std::minstd_rand0 &rng) {
    std::uniform_real_distribution<float> d(-1.f, 1.f);
    glm::vec3 v = {d(rng), d(rng), d(rng)};
    v = glm::normalize(v);
    if (glm::dot(v, normal) < 0)
        return -v;
    else
        return v;
}

glm::vec3 Scene::get_color(Ray ray, int depth, std::minstd_rand0 &rng) const {
    if (depth == 0)
        return {0.f, 0.f, 0.f};

    auto [insc, p_obj] = intersect(ray);
    if (p_obj == nullptr)
        return bg_color;

    switch (p_obj->material) {
    case Material::Diffuse: {
        Ray new_ray = {ray.at(insc.value().t), get_random_reflection(insc.value().normal, rng)};
        auto new_color = get_color(new_ray.step(), depth - 1, rng);
        return p_obj->emission + 2.f * p_obj->color * new_color * glm::dot(new_ray.dir, insc.value().normal);
    }
    case Material::Metallic: {
        Ray new_ray = {ray.at(insc.value().t), glm::reflect(ray.dir, insc.value().normal)};
        auto new_color = get_color(new_ray.step(), depth - 1, rng);
        return p_obj->color * new_color;
    }
    case Material::Dielectric: {
        float eta1 = insc.value().inside ? p_obj->dielectric_ior : 1.f;
        float eta2 = insc.value().inside ? 1.f : p_obj->dielectric_ior;
        float eta = eta1 / eta2;

        float cos_theta = glm::dot(-ray.dir, insc.value().normal);
        float R0 = std::pow((eta1 - eta2) / (eta1 + eta2), 2.f);
        float r = R0 + (1 - R0) * std::pow((1 - cos_theta), 5.f);

        float sin_theta2 = eta1 / eta2 * std::sqrt(1 - std::pow(cos_theta, 2));

        if (!(std::abs(sin_theta2) > 1) && std::bernoulli_distribution(r)(rng)) {
            Ray reflected_ray = {ray.at(insc.value().t), glm::reflect(ray.dir, insc.value().normal)};
            return get_color(reflected_ray.step(), depth - 1, rng);
        } else {
            Ray refracted_ray = {ray.at(insc.value().t), glm::refract(ray.dir, insc.value().normal, eta)};
            glm::vec3 refracted_color = get_color(refracted_ray.step(), depth - 1, rng);
            if (!insc.value().inside)
                refracted_color *= p_obj->color;
            return refracted_color;
        }
    }
    default:
        assert(false);
    }
}

} // namespace raytracing