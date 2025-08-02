#include <metatron/scene/daemon/camera.hpp>
#include <metatron/scene/compo/camera.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/lens/pinhole.hpp>
#include <metatron/render/lens/thin.hpp>
#include <metatron/core/math/sampler/sampler.hpp>
#include <metatron/core/math/sampler/independent.hpp>
#include <metatron/core/math/sampler/halton.hpp>
#include <metatron/core/math/filter/filter.hpp>
#include <metatron/core/math/filter/box.hpp>
#include <metatron/core/math/filter/gaussian.hpp>
#include <metatron/core/math/filter/lanczos.hpp>

namespace mtt::daemon {
	auto Camera_Daemon::update(ecs::Hierarchy& hierarchy) noexcept -> void {
		auto& registry = hierarchy.registry;
		auto camera_view = registry.view<ecs::Dirty_Mark<compo::Camera>>();
		for (auto entity: camera_view) {
			registry.remove<
				photo::Camera,
				photo::Film,
				poly<photo::Lens>,
				poly<math::Sampler>,
				poly<math::Filter>
			>(entity);
			if (!registry.any_of<compo::Camera>(entity)) {
				continue;
			}
			camera_entity = entity;
			auto& camera = registry.get<compo::Camera>(entity);

			registry.emplace<poly<math::Filter>>(entity,
			std::visit([&](auto&& compo) -> poly<math::Filter> {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Box_Filter>) {
					return make_poly<math::Filter>(math::Box_Filter{compo.radius});
				} else if constexpr (std::is_same_v<T, compo::Gaussian_Filter>) {
					return make_poly<math::Filter>(math::Gaussian_Filter{compo.radius, compo.sigma});
				} else if constexpr (std::is_same_v<T, compo::Lanczos_Filter>) {
					return make_poly<math::Filter>(math::Lanczos_Filter{compo.radius, compo.tau});
				}
			}, camera.filter));
			auto filter = view<math::Filter>{registry.get<poly<math::Filter>>(entity)};

			auto rd = std::random_device{};
			auto seed = rd();
			registry.emplace<poly<math::Sampler>>(entity,
			std::visit([&](auto&& compo) -> poly<math::Sampler> {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Independent_Sampler>) {
					return make_poly<math::Sampler>(math::Independent_Sampler{seed});
				} else if constexpr (std::is_same_v<T, compo::Halton_Sampler>) {
					return make_poly<math::Sampler>(math::Halton_Sampler{seed});
				}
			}, camera.sampler));
			auto sampler = view<math::Sampler>{registry.get<poly<math::Sampler>>(entity)};

			registry.emplace<poly<photo::Lens>>(entity,
			std::visit([&](auto&& compo) -> poly<photo::Lens> {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Pinhole_Lens>) {
					return make_poly<photo::Pinhole_Lens>(compo.focal_length);
				} else if constexpr (std::is_same_v<T, compo::Thin_Lens>) {
					return make_poly<photo::Thin_Lens>(compo.aperture, compo.focal_length, compo.focus_distance);
				}
			}, camera.lens));
			auto lens = view<photo::Lens>{registry.get<poly<photo::Lens>>(entity)};
			
			auto* color_space = compo::to_color_space(camera.color_space);
			registry.emplace<photo::Sensor>(entity, color_space);
			auto* sensor = &registry.get<photo::Sensor>(entity);
			auto x = photo::Film{camera.film_size, camera.image_size, filter, sensor, color_space};
			registry.emplace<photo::Film>(entity,
				camera.film_size,
				camera.image_size,
				filter,
				sensor,
				color_space
			);
			auto* film = &registry.get<photo::Film>(entity);
			registry.emplace<photo::Camera>(entity, lens, film);

			registry.clear<ecs::Dirty_Mark<compo::Camera>>();
		}
	}
}
