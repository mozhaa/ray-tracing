#include "scene.hpp"

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#ifdef MULTITHREADING_ENABLED
#include "thread_pool/BS_thread_pool.hpp"
#endif

#include "color.hpp"
#include "image.hpp"

namespace raytracing {

Scene::Scene(std::string fp) : camera(), objects(), ambient() {
    std::ifstream file(fp);
    std::string line;

    std::shared_ptr<Object> object(nullptr);
    std::shared_ptr<LightSource> light(nullptr);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        if (command == "DIMENSIONS") {
            iss >> camera.width >> camera.height;
        } else if (command == "BG_COLOR") {
            iss >> bg_color.x >> bg_color.y >> bg_color.z;
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
        } else if (command == "AMBIENT_LIGHT") {
            iss >> ambient.color.x >> ambient.color.y >> ambient.color.z;
        } else if (command == "NEW_LIGHT") {
            if (light.get() != nullptr)
                light_sources.push_back(light);
            light = std::shared_ptr<LightSource>(new LightSource());
        } else if (command == "LIGHT_INTENSITY") {
            iss >> light->intensity.x >> light->intensity.y >> light->intensity.z;
        } else if ((command == "LIGHT_DIRECTION") || (command == "LIGHT_DIR")) {
            light->type = LightSourceType::Direct;
            iss >> light->direction.x >> light->direction.y >> light->direction.z;
            light->direction = glm::normalize(light->direction);
        } else if ((command == "LIGHT_POSITION") || (command == "LIGHT_POS")) {
            light->type = LightSourceType::Point;
            iss >> light->position.x >> light->position.y >> light->position.z;
        } else if (command == "LIGHT_ATTENUATION") {
            light->type = LightSourceType::Point;
            iss >> light->attenuation.x >> light->attenuation.y >> light->attenuation.z;
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
    if (light.get() != nullptr)
        light_sources.push_back(light);
}

void Scene::render(std::string fp) const {
    std::vector<Pixel> image_data(camera.width * camera.height);

    auto set_pixel = [&](int t) {
        auto j = t % camera.height;
        auto i = (t - j) / camera.height;
        auto ray = camera.get_ray(i, j);
        auto color = get_color(ray, ray_depth);
        image_data[i + j * camera.width] = aces_tonemap(color);
    };

#ifdef MULTITHREADING_ENABLED
    BS::thread_pool pool;
    pool.submit_loop(0, camera.width * camera.height, set_pixel, 8).wait();
#else
    for (int t = 0; t < camera.width * camera.height; t++)
        set_pixel(t);
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

glm::vec3 Scene::get_color(Ray ray, int depth) const {
    if (depth == 0)
        return {0.f, 0.f, 0.f};

    auto [insc, p_obj] = intersect(ray);
    if (p_obj == nullptr)
        return bg_color;

    switch (p_obj->material) {
    case Material::Diffuse: {
        glm::vec3 total_color = ambient.color;
        for (auto& light : light_sources) {
            auto [new_ray, max_distance] = light->where_to_look(ray.at(insc.value().t));
            auto [new_insc, new_p_obj] = intersect(new_ray.step(), max_distance);
            if (new_p_obj == nullptr) {
                total_color += light->at(ray.at(insc.value().t), insc.value().normal);
            }
        }
        return p_obj->color * total_color;
    }
    case Material::Metallic: {
        Ray new_ray = {ray.at(insc.value().t), glm::reflect(ray.dir, insc.value().normal)};
        auto new_color = get_color(new_ray.step(), depth - 1);
        return p_obj->color * new_color;
    }
    case Material::Dielectric: {
        float eta1 = insc.value().inside ? p_obj->dielectric_ior : 1.f;
        float eta2 = insc.value().inside ? 1.f : p_obj->dielectric_ior;
        float eta = eta1 / eta2;

        Ray refracted_ray = {ray.at(insc.value().t), glm::refract(ray.dir, insc.value().normal, eta)};
        auto refracted_color = get_color(refracted_ray.step(), depth - 1);
        
        Ray reflected_ray = {ray.at(insc.value().t), glm::reflect(ray.dir, insc.value().normal)};
        auto reflected_color = get_color(reflected_ray.step(), depth - 1);

        float cos_theta = glm::dot(-ray.dir, insc.value().normal);
        float R0 = std::pow((eta1 - eta2) / (eta1 + eta2), 2.f);
        float r = std::max(0.f, std::min(R0 + (1 - R0) * std::pow((1 - cos_theta), 5.f), 1.f));
        
        return r * reflected_color + (1 - r) * refracted_color;
    }
    default: {
        assert(false);
    }
    }
}

} // namespace raytracing