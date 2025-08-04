#include <metatron/scene/daemon/tracer.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/scene/compo/shape.hpp>
#include <metatron/scene/compo/medium.hpp>
#include <metatron/scene/compo/light.hpp>
#include <metatron/scene/compo/camera.hpp>
#include <metatron/scene/compo/tracer.hpp>
#include <metatron/resource/light/area.hpp>
#include <metatron/render/emitter/uniform.hpp>
#include <metatron/render/accel/lbvh.hpp>

namespace mtt::daemon {
	auto Tracer_Daemon::init() noexcept -> void {}

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
			auto& div = registry.get<compo::Divider>(entity);
			auto& shape = registry.get<poly<shape::Shape>>(
				registry.get<compo::Shape_Instance>(div.shape).path
			);
			auto& medium = registry.get<poly<media::Medium>>(
				registry.get<compo::Medium_Instance>(div.medium).path
			);
			auto& material = registry.get<material::Material>(div.material);
			auto& st = registry.get<math::Transform>(div.shape);
			auto& mt = registry.get<math::Transform>(div.medium);

			auto& areas = registry.emplace_or_replace<std::vector<light::Area_Light>>(entity);
			if (material.spectrum_textures.contains("emission")) {
				areas.reserve(shape->size());
				for (auto i = 0uz; i < shape->size(); i++) {
					areas.emplace_back(shape, i);
					lights.emplace_back(&areas[i], &st);
				}
			}

			auto divs = std::vector<accel::Divider>{};
			divs.reserve(shape->size());
			for (auto i = 0uz; i < shape->size(); i++) {
				divs.emplace_back(
					shape, medium, areas.empty() ? nullptr : &areas[i],
					&material, &st, &mt, i
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

			switch (tracer.emitter) {
				case compo::Emitter::uniform:
					registry.emplace_or_replace<poly<emitter::Emitter>>(entity,
						make_poly<emitter::Emitter, emitter::Uniform_Emitter>(std::move(lights), std::move(inf_lights))
					);
					break;
			}
			auto camera_space_view = registry.view<compo::Camera_Space>();
			if (camera_space_view.empty()) {
				std::println("tracer: camera not attached");
				std::abort();
			}
			auto& camera_space = registry.get<compo::Camera_Space>(camera_space_view.front());
			switch (tracer.accel) {
				case compo::Acceleration::lbvh:
					registry.emplace_or_replace<poly<accel::Acceleration>>(entity,
						make_poly<accel::Acceleration, accel::LBVH>(
							std::move(dividers),
							&camera_space.world_to_render
						)
					);
					break;
			}
		}
	}
}
