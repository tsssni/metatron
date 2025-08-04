#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/lambertian.hpp>
#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/resource/bsdf/microfacet.hpp>
#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/resource/media/vaccum.hpp>
#include <metatron/resource/media/grid.hpp>
#include <metatron/resource/media/nanovdb.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/resource/light/environment.hpp>
#include <metatron/resource/light/parallel.hpp>
#include <metatron/resource/light/point.hpp>
#include <metatron/resource/light/spot.hpp>
#include <metatron/resource/light/area.hpp>
#include <metatron/render/emitter/uniform.hpp>
#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/render/accel/lbvh.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/scene/ecs/stage.hpp>
#include <metatron/scene/compo/spectrum.hpp>
#include <metatron/scene/compo/transform.hpp>
#include <metatron/scene/compo/shape.hpp>
#include <metatron/scene/compo/texture.hpp>
#include <metatron/scene/compo/light.hpp>
#include <metatron/scene/compo/camera.hpp>
#include <metatron/scene/daemon/spectrum.hpp>
#include <metatron/scene/daemon/color-space.hpp>
#include <metatron/scene/daemon/transform.hpp>
#include <metatron/scene/daemon/shape.hpp>
#include <metatron/scene/daemon/texture.hpp>
#include <metatron/scene/daemon/light.hpp>
#include <metatron/scene/daemon/camera.hpp>
#include <atomic>
#include <print>

using namespace mtt;

auto main() -> int {
	auto hierarchy = ecs::Hierarchy{};
	auto stage = std::make_unique<ecs::Stage>();
	auto spectrum_daemon = daemon::Spectrum_Daemon{};
	auto color_space_daemon = daemon::Color_Space_Daemon{};
	auto transform_daemon = daemon::Transform_Daemon{};
	auto shape_daemon = daemon::Shape_Daemon{};
	auto texture_daemon = daemon::Texture_Daemon{};
	auto light_daemon = daemon::Light_Daemon{};
	auto camera_daemon = daemon::Camera_Daemon{};
	stage->daemons.push_back(&spectrum_daemon);
	stage->daemons.push_back(&color_space_daemon);
	stage->daemons.push_back(&transform_daemon);
	stage->daemons.push_back(&shape_daemon);
	stage->daemons.push_back(&texture_daemon);
	stage->daemons.push_back(&light_daemon);
	stage->daemons.push_back(&camera_daemon);
	hierarchy.stages.push_back(stage.get());
	hierarchy.activate();
	hierarchy.init();

	auto sRBB_entity = hierarchy.entity("/color-space/sRGB");
	auto* sRGB = &hierarchy.fetch<color::Color_Space>(sRBB_entity);

	hierarchy.attach(hierarchy.create("/render"), compo::Transform{
		.translation = {0.f, 0.f, 1000.f},
	});
	hierarchy.attach(hierarchy.create("/camera"),compo::Transform{
		.translation = {0.f, 0.f, 0.f},
	});
	hierarchy.attach(hierarchy.create("/hierarchy"), compo::Transform{});
	hierarchy.attach(hierarchy.create("/hierarchy/shape"), compo::Transform{});
	hierarchy.attach(hierarchy.create("/hierarchy/shape/bound"), compo::Transform{
		.scaling = {500.0f},
	});
	hierarchy.attach(hierarchy.create("/hierarchy/medium"), compo::Transform{});
	hierarchy.attach(hierarchy.create("/hierarchy/medium/cloud"), compo::Transform{
		.rotation = math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 1.f / 2.f),
	});
	hierarchy.attach(hierarchy.create("/hierarchy/light"), compo::Transform{});
	hierarchy.attach(hierarchy.create("/hierarchy/light/env"), compo::Transform{});

	hierarchy.attach<compo::Shape>(hierarchy.create("/shape/sphere"), compo::Sphere{});
	hierarchy.attach(hierarchy.entity("/hierarchy/shape/bound"), compo::Shape_Instance{
		.path = hierarchy.entity("/shape/sphere")
	});

	hierarchy.attach<compo::Spectrum>(hierarchy.create("/spectrum/sigma-a"), compo::Rgb_Spectrum{
		.c = {0.0f, 0.0f, 0.0f},
		.type = color::Color_Space::Spectrum_Type::unbounded,
		.color_space = sRBB_entity,
	});
	hierarchy.attach<compo::Spectrum>(hierarchy.create("/spectrum/sigma-s"), compo::Rgb_Spectrum{
		.c = {1.0f, 1.0f, 1.0f},
		.type = color::Color_Space::Spectrum_Type::unbounded,
		.color_space = sRBB_entity,
	});
	hierarchy.attach<compo::Spectrum>(hierarchy.create("/spectrum/sigma-e"), compo::Rgb_Spectrum{
		.c = {0.0f, 0.0f, 0.0f},
		.type = color::Color_Space::Spectrum_Type::illuminant,
		.color_space = sRBB_entity,
	});
	hierarchy.attach<compo::Spectrum>(hierarchy.create("/spectrum/env"), compo::Rgb_Spectrum{
		.c = {0.03f, 0.07f, 0.23f},
		.type = color::Color_Space::Spectrum_Type::illuminant,
		.color_space = sRBB_entity
	});

	hierarchy.attach<compo::Texture>(hierarchy.create("/texture/env-map"), compo::Image_Spectrum_Texture{
		.path = "../metatron-scenes/material/texture/sky-on-fire.exr",
		.type = color::Color_Space::Spectrum_Type::illuminant,
	});

	hierarchy.attach<compo::Light>(hierarchy.create("/hierarchy/light/env"), compo::Environment_Light{
		.env_map = hierarchy.entity("/texture/env-map"),
	});

	hierarchy.attach(hierarchy.entity("/camera"), compo::Camera{
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

	auto vaccum_medium = media::Vaccum_Medium{};
	auto hg_phase = phase::Henyey_Greenstein_Phase_Function{0.877f};
	auto nanovdb_grid = media::Nanovdb_Grid<
		f32,
		media::grid_size,
		media::grid_size,
		media::grid_size
	>{"../metatron-scenes/disney-cloud/volume/disney-cloud.nvdb"};
	auto cloud_medium = media::Grid_Medium{
		&nanovdb_grid,
		make_poly<phase::Phase_Function, phase::Henyey_Greenstein_Phase_Function>(0.877f),
		hierarchy.fetch<poly<spectra::Spectrum>>(hierarchy.entity("/spectrum/sigma-a")),
		hierarchy.fetch<poly<spectra::Spectrum>>(hierarchy.entity("/spectrum/sigma-s")),
		hierarchy.fetch<poly<spectra::Spectrum>>(hierarchy.entity("/spectrum/sigma-e")),
		1.f,
	};

	auto interface_material = material::Material{
		.bsdf = make_poly<bsdf::Bsdf, bsdf::Interface_Bsdf>(),
	};

	auto lights = std::vector<emitter::Divider>{};
	auto inf_lights = std::vector<emitter::Divider>{
		{
			hierarchy.fetch<poly<light::Light>>(hierarchy.entity("/hierarchy/light/env")),
			&hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/light/env"))
		},
	};
	auto emitter = emitter::Uniform_Emitter{std::move(lights), std::move(inf_lights)};

	auto dividers = std::vector<accel::Divider>{
		{
			.shape = hierarchy.fetch<poly<shape::Shape>>(
				hierarchy.fetch<compo::Shape_Instance>(
					hierarchy.entity("/hierarchy/shape/bound")
				).path
			),
			.medium = &cloud_medium,
			.light = nullptr,
			.material = &interface_material,
			.local_to_world = &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/shape/bound")),
			.medium_to_world = &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/medium/cloud")),
		},
	};

	auto& identity = hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy"));
	auto& world_to_render = hierarchy.fetch<math::Transform>(hierarchy.entity("/render"));
	auto& render_to_camera = hierarchy.fetch<math::Transform>(hierarchy.entity("/camera"));
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
					&vaccum_medium,
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
