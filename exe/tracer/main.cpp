#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/render/emitter/uniform.hpp>
#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/render/accel/lbvh.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/compo/spectrum.hpp>
#include <metatron/scene/compo/transform.hpp>
#include <metatron/scene/compo/shape.hpp>
#include <metatron/scene/compo/medium.hpp>
#include <metatron/scene/compo/texture.hpp>
#include <metatron/scene/compo/material.hpp>
#include <metatron/scene/compo/light.hpp>
#include <metatron/scene/compo/camera.hpp>
#include <metatron/scene/daemon/spectrum.hpp>
#include <metatron/scene/daemon/color-space.hpp>
#include <metatron/scene/daemon/transform.hpp>
#include <metatron/scene/daemon/shape.hpp>
#include <metatron/scene/daemon/medium.hpp>
#include <metatron/scene/daemon/light.hpp>
#include <metatron/scene/daemon/texture.hpp>
#include <metatron/scene/daemon/material.hpp>
#include <metatron/scene/daemon/camera.hpp>
#include <metatron/core/stl/thread.hpp>
#include <atomic>
#include <print>
#include <iostream>

using namespace mtt;

auto main() -> int {
	auto hierarchy = ecs::Hierarchy{};

	auto spectrum_stage = std::make_unique<ecs::Stage>();
	auto resource_stage = std::make_unique<ecs::Stage>();
	auto material_stage = std::make_unique<ecs::Stage>();
	auto render_stage = std::make_unique<ecs::Stage>();

	auto spectrum_daemon = daemon::Spectrum_Daemon{};
	auto color_space_daemon = daemon::Color_Space_Daemon{};
	auto transform_daemon = daemon::Transform_Daemon{};
	auto shape_daemon = daemon::Shape_Daemon{};
	auto medium_daemon = daemon::Medium_Daemon{};
	auto texture_daemon = daemon::Texture_Daemon{};
	auto light_daemon = daemon::Light_Daemon{};
	auto material_daemon = daemon::Material_Daemon{};
	auto camera_daemon = daemon::Camera_Daemon{};

	spectrum_stage->daemons = {
		&spectrum_daemon,
		&color_space_daemon,
	};
	resource_stage->daemons = {
		&transform_daemon,
		&shape_daemon,
		&medium_daemon,
		&texture_daemon,
	};
	material_stage->daemons = {
		&light_daemon,
		&material_daemon,
	};
	render_stage->daemons = {
		&camera_daemon,
	};
	hierarchy.stages = {
		spectrum_stage.get(),
		resource_stage.get(),
		material_stage.get(),
		render_stage.get()
	};
	hierarchy.activate();
	hierarchy.init();

	auto sRBB_entity = "/color-space/sRGB"_et;
	auto* sRGB = &hierarchy.fetch<color::Color_Space>(sRBB_entity);

	hierarchy.attach<compo::Spectrum>("/spectrum/sigma-a"_et, compo::Rgb_Spectrum{
		.c = {0.0f, 0.0f, 0.0f},
		.type = color::Color_Space::Spectrum_Type::unbounded,
		.color_space = sRBB_entity,
	});
	hierarchy.attach<compo::Spectrum>("/spectrum/sigma-s"_et, compo::Rgb_Spectrum{
		.c = {1.0f, 1.0f, 1.0f},
		.type = color::Color_Space::Spectrum_Type::unbounded,
		.color_space = sRBB_entity,
	});
	hierarchy.attach<compo::Spectrum>("/spectrum/sigma-e"_et, compo::Rgb_Spectrum{
		.c = {0.0f, 0.0f, 0.0f},
		.type = color::Color_Space::Spectrum_Type::illuminant,
		.color_space = sRBB_entity,
	});
	hierarchy.attach<compo::Spectrum>("/spectrum/env"_et, compo::Rgb_Spectrum{
		.c = {0.03f, 0.07f, 0.23f},
		.type = color::Color_Space::Spectrum_Type::illuminant,
		.color_space = sRBB_entity
	});

	hierarchy.attach("/render"_et, compo::Transform{
		.translation = {0.f, 0.f, 1000.f},
	});
	hierarchy.attach("/camera"_et,compo::Transform{
		.translation = {0.f, 0.f, 0.f},
	});
	hierarchy.attach("/hierarchy"_et, compo::Transform{});
	hierarchy.attach("/hierarchy/shape"_et, compo::Transform{});
	hierarchy.attach("/hierarchy/shape/bound"_et, compo::Transform{
		.scaling = {500.0f},
	});
	hierarchy.attach("/hierarchy/medium"_et, compo::Transform{});
	hierarchy.attach("/hierarchy/medium/cloud"_et, compo::Transform{
		.rotation = math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 1.f / 2.f),
	});
	hierarchy.attach("/hierarchy/light"_et, compo::Transform{});
	hierarchy.attach("/hierarchy/light/env"_et, compo::Transform{});

	hierarchy.attach<compo::Shape>("/shape/sphere"_et, compo::Sphere{});
	hierarchy.attach("/hierarchy/shape/bound"_et, compo::Shape_Instance{
		.path = "/shape/sphere"_et,
	});

	hierarchy.attach<compo::Medium>("/medium/vaccum"_et, compo::Vaccum_Medium{});
	hierarchy.attach("/hierarchy/medium/vaccum"_et, compo::Medium_Instance{
		.path = "/medium/vaccum"_et,
	});
	hierarchy.attach<compo::Medium>("/medium/cloud"_et, compo::Grid_Medium{
		.grid = "../metatron-scenes/disney-cloud/volume/disney-cloud.nvdb",
		.phase = compo::Henyey_Greenstein_Phase_Function{
			.g = 0.877f
		},
		.sigma_a = "/spectrum/sigma-a"_et,
		.sigma_s = "/spectrum/sigma-s"_et,
		.sigma_e = "/spectrum/sigma-e"_et,
		.density_scale = 1.f,
	});
	hierarchy.attach("/hierarchy/medium/cloud"_et, compo::Medium_Instance{
		.path = "/medium/cloud"_et,
	});

	hierarchy.attach<compo::Texture>("/texture/env-map"_et, compo::Image_Spectrum_Texture{
		.path = "../metatron-scenes/material/texture/sky-on-fire.exr",
		.type = color::Color_Space::Spectrum_Type::illuminant,
	});

	hierarchy.attach<compo::Light>("/hierarchy/light/env"_et, compo::Environment_Light{
		.env_map = "/texture/env-map"_et,
	});

	hierarchy.attach("/material/cloud"_et, compo::Material{
		.bsdf = compo::Bsdf::interface,
	});

	hierarchy.attach("/camera"_et, compo::Camera{
		.film_size = {0.036f, 0.024f},
		.image_size = {600uz, 400uz},
		.spp = 16,
		.depth = 64,
		.lens = compo::Thin_Lens{
			.aperture = 5.6f,
			.focal_length = 0.05f,
			.focus_distance = 10.f,
		},
		.sampler = compo::Halton_Sampler{},
		.filter = compo::Lanczos_Filter{},
		.color_space = sRBB_entity,
	});

	hierarchy.update();

	auto lights = std::vector<emitter::Divider>{};
	auto inf_lights = std::vector<emitter::Divider>{
		{
			hierarchy.fetch<poly<light::Light>>("/hierarchy/light/env"_et),
			&hierarchy.fetch<math::Transform>("/hierarchy/light/env"_et)
		},
	};
	auto emitter = emitter::Uniform_Emitter{std::move(lights), std::move(inf_lights)};

	auto dividers = std::vector<accel::Divider>{
		{
			.shape = hierarchy.fetch<poly<shape::Shape>>(
				hierarchy.fetch<compo::Shape_Instance>("/hierarchy/shape/bound"_et).path
			),
			.medium = hierarchy.fetch<poly<media::Medium>>(
				hierarchy.fetch<compo::Medium_Instance>("/hierarchy/medium/cloud"_et).path
			),
			.light = nullptr,
			.material = &hierarchy.fetch<material::Material>("/material/cloud"_et),
			.local_to_world = &hierarchy.fetch<math::Transform>("/hierarchy/shape/bound"_et),
			.medium_to_world = &hierarchy.fetch<math::Transform>("/hierarchy/medium/cloud"_et),
		},
	};

	auto& identity = hierarchy.fetch<math::Transform>("/hierarchy"_et);
	auto& world_to_render = hierarchy.fetch<math::Transform>("/render"_et);
	auto& render_to_camera = hierarchy.fetch<math::Transform>("/camera"_et);
	auto& vaccum_medium = hierarchy.fetch<poly<media::Medium>>(
		hierarchy.fetch<compo::Medium_Instance>("/hierarchy/medium/vaccum"_et).path
	);
	auto bvh = accel::LBVH{std::move(dividers), &world_to_render};
	auto integrator = monte_carlo::Volume_Path_Integrator{};

	auto atomic_count = std::atomic<usize>{0uz};
	auto sampler = mut<math::Sampler>{hierarchy.fetch<poly<math::Sampler>>(camera_daemon.camera_entity)};
	auto camera = mut<photo::Camera>{&hierarchy.fetch<photo::Camera>(camera_daemon.camera_entity)};
	auto size = hierarchy.fetch<photo::Film>(camera_daemon.camera_entity).image_size;
	auto spp = hierarchy.fetch<compo::Camera>(camera_daemon.camera_entity).spp;
	auto depth = hierarchy.fetch<compo::Camera>(camera_daemon.camera_entity).depth;
	auto total = size[0] * size[1] * spp;
	auto last_percent = -1;
	
	stl::scheduler::instance().sync_parallel(size, [&](math::Vector<usize, 2> const& px) {
		for (auto n = 0uz; n < spp; n++) {
			auto sample = camera->sample(px, n, sampler);
			sample->ray_differential = render_to_camera ^ sample->ray_differential;
			auto& s = sample.value();

			auto Li_opt = integrator.sample(
				{
					s.ray_differential,
					s.default_differential,
					&identity,
					&world_to_render,
					&render_to_camera,
					vaccum_medium,
					px,
					n,
					depth,
				},
				&bvh,
				&emitter,
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

	std::print("\n");
	camera->to_path("build/test.exr");
	return 0;
}
