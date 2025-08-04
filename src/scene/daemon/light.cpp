#include <metatron/scene/daemon/light.hpp>
#include <metatron/scene/compo/light.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/light/parallel.hpp>
#include <metatron/resource/light/point.hpp>
#include <metatron/resource/light/spot.hpp>
#include <metatron/resource/light/area.hpp>
#include <metatron/resource/light/environment.hpp>

namespace mtt::daemon {
	auto Light_Daemon::init() noexcept -> void {}

	auto Light_Daemon::update() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;
		auto light_view = registry.view<ecs::Dirty_Mark<compo::Light>>();
		for (auto entity: light_view) {
			registry.remove<poly<light::Light>>(entity);
			if (!registry.any_of<compo::Light>(entity)) {
				continue;
			}
			auto& light = registry.get<compo::Light>(entity);

			registry.emplace<poly<light::Light>>(entity,
			std::visit([&](auto&& compo) -> poly<light::Light> {
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
				} else if constexpr (std::is_same_v<T, compo::Area_Light>) {
					return make_poly<light::Light, light::Area_Light>(
						registry.get<poly<shape::Shape>>(compo.shape),
						compo.primitive
					);
				} else if constexpr (std::is_same_v<T, compo::Environment_Light>) {
					return make_poly<light::Light, light::Environment_Light>(
						registry.get<poly<texture::Texture<spectra::Stochastic_Spectrum>>>(compo.env_map)
					);
				}
			},light));
		}
		registry.clear<ecs::Dirty_Mark<compo::Light>>();
	}
}
