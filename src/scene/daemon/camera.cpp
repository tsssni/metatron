#include <metatron/scene/daemon/camera.hpp>
#include <metatron/scene/compo/camera.hpp>
#include <metatron/scene/compo/transform.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/scene/serde/serde.hpp>
#include <metatron/resource/photo/camera.hpp>
#include <metatron/resource/lens/pinhole.hpp>
#include <metatron/resource/lens/thin.hpp>
#include <metatron/core/math/sampler/sampler.hpp>
#include <metatron/core/math/sampler/independent.hpp>
#include <metatron/core/math/sampler/halton.hpp>
#include <metatron/core/math/filter/filter.hpp>
#include <metatron/core/math/filter/box.hpp>
#include <metatron/core/math/filter/gaussian.hpp>
#include <metatron/core/math/filter/lanczos.hpp>

namespace mtt::daemon {
	auto Camera_Daemon::init() noexcept -> void {
		MTT_SERDE(Camera);
	}

	auto Camera_Daemon::update() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;

		auto entities = registry.view<ecs::Dirty_Mark<compo::Camera>>();
		for (auto entity: entities) {
			auto remove_camera = [&registry](ecs::Entity entity) {
				registry.remove<
					compo::Camera_Space,
					photo::Camera,
					photo::Film,
					poly<photo::Lens>,
					poly<math::Sampler>,
					poly<math::Filter>
				>(entity);
			};
			remove_camera(entity);
			if (this->camera != ecs::null) {
				remove_camera(this->camera);
				registry.remove<compo::Camera>(this->camera);
			}
			if (!registry.any_of<compo::Camera>(entity)) {
				continue;
			}
			this->camera = entity;
			auto& camera = registry.get<compo::Camera>(entity);

			registry.emplace<poly<math::Filter>>(entity,
			std::visit([&](auto&& compo) -> poly<math::Filter> {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Box_Filter>) {
					return make_poly<math::Filter, math::Box_Filter>(compo.radius);
				} else if constexpr (std::is_same_v<T, compo::Gaussian_Filter>) {
					return make_poly<math::Filter, math::Gaussian_Filter>(compo.radius, compo.sigma);
				} else if constexpr (std::is_same_v<T, compo::Lanczos_Filter>) {
					return make_poly<math::Filter, math::Lanczos_Filter>(compo.radius, compo.tau);
				}
			}, camera.filter));
			auto filter = view<math::Filter>{registry.get<poly<math::Filter>>(entity)};

			auto rd = std::random_device{};
			auto seed = rd();
			registry.emplace<poly<math::Sampler>>(entity,
			std::visit([&](auto&& compo) -> poly<math::Sampler> {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Independent_Sampler>) {
					return make_poly<math::Sampler, math::Independent_Sampler>(seed);
				} else if constexpr (std::is_same_v<T, compo::Halton_Sampler>) {
					return make_poly<math::Sampler, math::Halton_Sampler>(seed);
				}
			}, camera.sampler));
			auto sampler = view<math::Sampler>{registry.get<poly<math::Sampler>>(entity)};

			registry.emplace<poly<photo::Lens>>(entity,
			std::visit([&](auto&& compo) -> poly<photo::Lens> {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Pinhole_Lens>) {
					return make_poly<photo::Lens, photo::Pinhole_Lens>(
						compo.focal_distance
					);
				} else if constexpr (std::is_same_v<T, compo::Thin_Lens>) {
					return make_poly<photo::Lens, photo::Thin_Lens>(
						compo.aperture, compo.focal_length, compo.focus_distance
					);
				}
			}, camera.lens));
			auto lens = view<photo::Lens>{registry.get<poly<photo::Lens>>(entity)};
			
			auto* color_space = &registry.get<color::Color_Space>(camera.color_space);
			registry.emplace<photo::Sensor>(entity, color_space);
			auto* sensor = &registry.get<photo::Sensor>(entity);

			auto film_size = camera.film_size;
			auto image_size = camera.image_size;
			auto aspect_ratio = f32(camera.image_size[0]) / f32(camera.image_size[1]);
			if (aspect_ratio > 1.f) {
				film_size[1] = film_size[0] / aspect_ratio;
			} else {
				film_size[0] = film_size[1] * aspect_ratio;
			}
			registry.emplace<photo::Film>(entity,
				film_size,
				image_size,
				filter,
				sensor,
				color_space
			);
			auto* film = &registry.get<photo::Film>(entity);
			registry.emplace<photo::Camera>(entity, lens, film);

			auto& camera_space = registry.emplace_or_replace<compo::Camera_Space>(entity);
			auto camera_to_world = compo::to_transform(registry.get<compo::Transform>(entity));
			auto t = camera_to_world.transform;
			auto inv_t = camera_to_world.inv_transform;
			camera_space.world_to_render = compo::to_transform(compo::Local_Transform{
				.translation = -math::Vector<f32, 3>{t[0][3], t[1][3], t[2][3]},
			});
			t[0][3] = t[1][3] = t[2][3] = 0.0f;
			inv_t[0][3] = inv_t[1][3] = inv_t[2][3] = 0.0f;
			camera_space.render_to_camera.inv_transform = t;
			camera_space.render_to_camera.transform = inv_t;

			registry.clear<ecs::Dirty_Mark<compo::Camera>>();
		}
	}
}
