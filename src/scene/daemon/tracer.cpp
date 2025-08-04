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
#include <metatron/core/math/sampler/sampler.hpp>
#include <atomic>
#include <print>
#include <iostream>

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
			registry.remove<std::vector<light::Area_Light>>(entity);
			if (!registry.any_of<compo::Divider>(entity)) {
				continue;
			}
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

			auto& areas = registry.emplace<std::vector<light::Area_Light>>(entity);
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
					registry.emplace<poly<emitter::Emitter>>(entity,
						make_poly<emitter::Emitter, emitter::Uniform_Emitter>(std::move(lights), std::move(inf_lights))
					);
					break;
			}
			auto camera_space_view = registry.view<compo::Camera_Space>();
			if (camera_space_view.empty()) {
				std::println("tracer: camera not attached");
				std::abort();
			}
			auto& space = registry.get<compo::Camera_Space>(camera_space_view.front());
			switch (tracer.accel) {
				case compo::Acceleration::lbvh:
					registry.emplace<poly<accel::Acceleration>>(entity,
						make_poly<accel::Acceleration, accel::LBVH>(
							std::move(dividers),
							&space.world_to_render
						)
					);
					break;
			}
		}
	}

	auto Tracer_Daemon::render(std::string_view path) noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;

		auto camera_view = registry.view<compo::Camera>();
		if (camera_view->empty()) {
			std::println("tracer: camera not attached");
			std::abort();
		}
		auto entity = camera_view.front();
		auto& compo = registry.get<compo::Camera>(entity);
		auto& space = registry.get<compo::Camera_Space>(entity);
		auto& camera = registry.get<photo::Camera>(entity);
		auto& sampler = registry.get<poly<math::Sampler>>(entity);

		auto integrator = [](compo::Integrator integrator) {
			switch (integrator) {
				case compo::Integrator::volume_path:
					return make_poly<monte_carlo::Integrator, monte_carlo::Volume_Path_Integrator>();
			}
		}(registry.get<compo::Tracer>(tracer).integrator);
		auto& accel = registry.get<poly<accel::Acceleration>>(tracer);
		auto& emitter = registry.get<poly<emitter::Emitter>>(tracer);

		auto atomic_count = std::atomic<usize>{0uz};
		auto total = compo.image_size[0] * compo.image_size[1] * compo.spp;
		auto last_percent = -1;

		stl::scheduler::instance().sync_parallel(compo.image_size, [&](math::Vector<usize, 2> const& px) {
			for (auto n = 0uz; n < compo.spp; n++) {
				auto sample = camera.sample(px, n, sampler);
				sample->ray_differential = space.render_to_camera ^ sample->ray_differential;
				auto& s = sample.value();

				auto Li_opt = integrator->sample(
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
					sampler
				);

				auto& Li = Li_opt.value();
				s.fixel = Li;
				auto count = atomic_count.fetch_add(1) + 1;
				auto percent = static_cast<int>(100.f * count / total);
				if (percent > last_percent) {
					last_percent = percent;
					std::print("\rprogress: {}%", percent);
					std::flush(std::cout);
				}
			}
		});
		std::println();
		camera.to_path(path);
	}
}
