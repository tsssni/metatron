#include <metatron/scene/daemon/light.hpp>
#include <metatron/scene/compo/light.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/scene/serde/serde.hpp>
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/light/parallel.hpp>
#include <metatron/resource/light/point.hpp>
#include <metatron/resource/light/spot.hpp>
#include <metatron/resource/light/area.hpp>
#include <metatron/resource/light/environment.hpp>
#include <metatron/resource/light/sunsky.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::daemon {
    auto Light_Daemon::init() noexcept -> void {
        MTT_SERDE(Light);
        light::Sunsky_Light::init();
    }

    auto Light_Daemon::update() noexcept -> void {
        auto& hierarchy = *ecs::Hierarchy::instance;
        auto& registry = hierarchy.registry;

        auto view = registry.view<ecs::Dirty_Mark<compo::Light>>()
        | std::views::filter([&](auto entity) {
            registry.remove<poly<compo::Light>>(entity);
            return registry.any_of<compo::Light>(entity);
        })
        | std::ranges::to<std::vector<ecs::Entity>>();

        auto mutex = std::mutex{};
        stl::scheduler::instance().sync_parallel(math::Vector<usize, 1>{view.size()}, [&](auto idx) {
            auto entity = view[idx[0]];
            auto& light = registry.get<compo::Light>(entity);
            
            auto l = std::visit([&](auto&& compo) -> poly<light::Light> {
                using T = std::decay_t<decltype(compo)>;
                if constexpr (std::is_same_v<T, compo::Parallel_Light>) {
                    return make_poly<light::Light, light::Parallel_Light>(
                        registry.get<poly<spectra::Spectrum>>(compo.spectrum)
                    );
                } else if constexpr (std::is_same_v<T, compo::Point_Light>) {
                    return make_poly<light::Light, light::Point_Light>(
                        registry.get<poly<spectra::Spectrum>>(compo.spectrum)
                    );
                } else if constexpr (std::is_same_v<T, compo::Spot_Light>) {
                    return make_poly<light::Light, light::Spot_Light>(
                        registry.get<poly<spectra::Spectrum>>(compo.spectrum),
                        compo.falloff_start_theta,
                        compo.falloff_end_theta
                    );
                } else if constexpr (std::is_same_v<T, compo::Environment_Light>) {
                    if (!registry.any_of<poly<texture::Sampler>>(compo.sampler)) {
                        std::println(
                            "no sampler for env map {} in entity {}",
                            ecs::to_path(compo.env_map), ecs::to_path(compo.sampler)
                        );
                        std::abort();
                    }
                    return make_poly<light::Light, light::Environment_Light>(
                        registry.get<poly<texture::Spectrum_Texture>>(compo.env_map),
                        registry.get<poly<texture::Sampler>>(compo.sampler).get()
                    );
                } else if constexpr (std::is_same_v<T, compo::Sunsky_Light>) {
                    return make_poly<light::Light, light::Sunsky_Light>(
                        compo.direction,
                        compo.turbidity,
                        compo.albedo,
                        compo.aperture,
                        compo.temperature,
                        compo.intensity
                    );
                }
            },light);

            {
                auto lock = std::lock_guard(mutex);
                registry.emplace<poly<light::Light>>(entity, std::move(l));
            }
        });
        // clear after emitter construction
    }
}
