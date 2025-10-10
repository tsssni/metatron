#include <metatron/scene/daemon/tracer.hpp>
#include <metatron/scene/compo/shape.hpp>
#include <metatron/scene/compo/medium.hpp>
#include <metatron/scene/compo/light.hpp>
#include <metatron/scene/compo/camera.hpp>
#include <metatron/scene/compo/tracer.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/light/area.hpp>
#include <metatron/resource/photo/camera.hpp>
#include <metatron/render/emitter/uniform.hpp>
#include <metatron/render/accel/lbvh.hpp>
#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/stl/progress.hpp>

namespace mtt::daemon {
    auto Tracer_Daemon::init() noexcept -> void {
        MTT_SERDE(Divider);
        MTT_SERDE(Tracer);
    }

    auto Tracer_Daemon::update() noexcept -> void {
        auto& hierarchy = *ecs::Hierarchy::instance;
        auto& registry = hierarchy.registry;

        auto lights = std::vector<emitter::Divider>{};
        auto inf_lights = std::vector<emitter::Divider>{};
        auto light_view = registry.view<ecs::Dirty_Mark<compo::Light>>();
        for (auto entity: light_view) {
            auto& light = registry.get<poly<light::Light>>(entity);
            auto& t = registry.get<math::Transform>(entity);
            if (light->flags() & light::Flags::inf) {
                inf_lights.emplace_back(light, &t);
            } else {
                lights.emplace_back(light, &t);
            }
        }
        registry.clear<ecs::Dirty_Mark<compo::Light>>();

        auto dividers = std::vector<accel::Divider>{};
        auto div_view = registry.view<ecs::Dirty_Mark<compo::Divider>>();
        for (auto entity: div_view) {
            registry.remove<std::vector<light::Area_Light>>(entity);
            if (!registry.any_of<compo::Divider>(entity)) {
                continue;
            }
            auto& div = registry.get<compo::Divider>(entity);

            auto& shape = registry.get<poly<shape::Shape>>(
                registry.get<compo::Shape_Instance>(div.shape).path
            );
            auto& int_medium = registry.get<poly<media::Medium>>(
                registry.get<compo::Medium_Instance>(div.int_medium).path
            );
            auto& ext_medium = registry.get<poly<media::Medium>>(
                registry.get<compo::Medium_Instance>(div.ext_medium).path
            );
            auto& material = registry.get<material::Material>(div.material);
            auto& st = registry.get<math::Transform>(div.shape);
            auto& int_mt = registry.get<math::Transform>(div.int_medium);
            auto& ext_mt = registry.get<math::Transform>(div.ext_medium);

            auto& areas = registry.emplace<std::vector<light::Area_Light>>(entity);
            if (material.spectrum_textures.contains("emission")) {
                areas.reserve(shape->size());
                for (auto i = 0uz; i < shape->size(); ++i) {
                    areas.emplace_back(shape, i);
                    lights.emplace_back(&areas[i], &st);
                }
            }

            auto divs = std::vector<accel::Divider>{};
            divs.reserve(shape->size());
            for (auto i = 0uz; i < shape->size(); ++i) {
                divs.emplace_back(
                    shape, int_medium, ext_medium,
                    areas.empty() ? nullptr : &areas[i],
                    &material, &st, &int_mt, &ext_mt, i
                );
            }
            std::ranges::move(divs, std::back_inserter(dividers));
        }
        registry.clear<ecs::Dirty_Mark<compo::Divider>>();

        auto tracer_view = registry.view<ecs::Dirty_Mark<compo::Tracer>>();
        for (auto entity: tracer_view) {
            auto remove_tracer = [&registry](ecs::Entity entity) {
                registry.remove<
                    poly<emitter::Emitter>,
                    poly<accel::Acceleration>
                >(entity);
            };
            remove_tracer(entity);
            if (this->tracer != ecs::null) {
                remove_tracer(this->tracer);
                registry.remove<compo::Tracer>(this->tracer);
            }
            if (!registry.any_of<compo::Tracer>(entity)) {
                continue;
            }
            this->tracer = entity;
            auto& tracer = registry.get<compo::Tracer>(entity);

            registry.emplace<poly<emitter::Emitter>>(entity,
            std::visit([&](auto&& compo) {
                using T = std::decay_t<decltype(compo)>;
                if constexpr (std::is_same_v<T, compo::Uniform_Emitter>) {
                    return make_poly<emitter::Emitter, emitter::Uniform_Emitter>(std::move(lights), std::move(inf_lights));
                }
            }, tracer.emitter));
            
            auto camera_space_view = registry.view<compo::Camera_Space>();
            if (camera_space_view.empty()) {
                std::println("tracer: camera space not attached");
                std::abort();
            }
            auto& space = registry.get<compo::Camera_Space>(camera_space_view.front());
            
            registry.emplace<poly<accel::Acceleration>>(entity,
            std::visit([&](auto&& compo) {
                using T = std::decay_t<decltype(compo)>;
                if constexpr (std::is_same_v<T, compo::LBVH>) {
                    return make_poly<accel::Acceleration, accel::LBVH>(
                        std::move(dividers), &space.world_to_render
                    );
                }
            }, tracer.accel));
        }
    }

    auto Tracer_Daemon::render(
        std::string_view path,
        std::string_view address
    ) noexcept -> void {
        auto& hierarchy = *ecs::Hierarchy::instance;
        auto& registry = hierarchy.registry;

        auto camera_view = registry.view<compo::Camera>();
        if (camera_view.empty()) {
            std::println("tracer: camera not attached");
            std::abort();
        }
        auto entity = camera_view.front();
        auto& compo = registry.get<compo::Camera>(entity);
        auto& space = registry.get<compo::Camera_Space>(entity);
        auto& camera = registry.get<photo::Camera>(entity);
        auto& sampler = registry.get<poly<math::Sampler>>(entity);

        auto integrator = std::visit([&](auto&& compo) {
            using T = std::decay_t<decltype(compo)>;
            if constexpr (std::is_same_v<T, compo::Volume_Path_Integrator>) {
                return make_poly<monte_carlo::Integrator, monte_carlo::Volume_Path_Integrator>();
            }
        }, registry.get<compo::Tracer>(tracer).integrator);
        auto& accel = registry.get<poly<accel::Acceleration>>(tracer);
        auto& emitter = registry.get<poly<emitter::Emitter>>(tracer);

        auto range = math::Vector<usize, 2>{};
        auto next = 1uz;
        auto progress = stl::progress{compo.image_size[0] * compo.image_size[1] * compo.spp};
        auto trace = [&](math::Vector<usize, 2> const& px) {
            for (auto n = range[0]; n < range[1]; ++n) {
                auto sp = sampler;
                MTT_OPT_OR_CALLBACK(s, camera.sample(px, n, sp), {
                    std::println("ray generation failed");
                    std::abort();
                });
                s.ray_differential = space.render_to_camera ^ s.ray_differential;
                s.default_differential = space.render_to_camera ^ s.default_differential;

                MTT_OPT_OR_CALLBACK(Li, integrator->sample(
                    {
                        s.ray_differential,
                        s.default_differential,
                        &registry.get<math::Transform>(compo.initial_medium),
                        &space.world_to_render,
                        &space.render_to_camera,
                        registry.get<poly<media::Medium>>(registry.get<compo::Medium_Instance>(compo.initial_medium).path),
                        px,
                        n,
                        compo.depth,
                    },
                    accel,
                    emitter,
                    sp
                ), {
                    std::println("invalid value appears in pixel {} sample {}", px, n);
                    std::abort();
                });

                s.fixel = Li;
                ++progress;
            }
        };

        while (range[0] < compo.spp) {
            stl::scheduler::instance().sync_parallel(compo.image_size, trace);
            range[0] = range[1];
            range[1] = std::min(compo.spp, range[1] + next);
            next = std::min(next * 2uz, 64uz);

            auto image = camera.film->image;
            stl::scheduler::instance().sync_parallel(
                math::Vector<usize, 2>{image.size},
                [&image](math::Vector<usize, 2> const& px) {
                    auto [i, j] = px;
                    auto pixel = math::Vector<f32, 4>{image[i, j]};
                    pixel /= pixel[3];
                    image[i, j] = pixel;
                }
            );
            if (range[0] == compo.spp) {
                image.to_path(path);
            }
        }
        ~progress;
    }
}
