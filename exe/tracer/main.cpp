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
#include <metatron/scene/compo/camera.hpp>
#include <metatron/scene/daemon/spectrum.hpp>
#include <metatron/scene/daemon/color-space.hpp>
#include <metatron/scene/daemon/transform.hpp>
#include <metatron/scene/daemon/shape.hpp>
#include <metatron/scene/daemon/texture.hpp>
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
	auto camera_daemon = daemon::Camera_Daemon{};
	stage->daemons.push_back(&spectrum_daemon);
	stage->daemons.push_back(&color_space_daemon);
	stage->daemons.push_back(&transform_daemon);
	stage->daemons.push_back(&shape_daemon);
	stage->daemons.push_back(&texture_daemon);
	stage->daemons.push_back(&camera_daemon);
	hierarchy.stages.push_back(stage.get());
	hierarchy.activate();
	hierarchy.init();

	auto sRBB_entity = hierarchy.entity("/color-space/sRGB");
	auto* sRGB = &hierarchy.fetch<color::Color_Space>(sRBB_entity);

	hierarchy.attach(hierarchy.create("/render"), compo::Transform{
		.translation = {0.f, 0.f, 1000.f},
		// .translation = math::Transform{{-2.5f, -0.6f, 2.5f}};
	});
	hierarchy.attach(hierarchy.create("/camera"),compo::Transform{
		.translation = {0.f, 0.f, 0.f},
		// .rotation = math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 1.f / 4.f),
	});
	hierarchy.attach(hierarchy.create("/hierarchy"), compo::Transform{});
	hierarchy.attach(hierarchy.create("/hierarchy/shape"), compo::Transform{});
	hierarchy.attach(hierarchy.create("/hierarchy/shape/sphere"), compo::Transform{
		.scaling = {200.f},
	});
	hierarchy.attach(hierarchy.create("/hierarchy/shape/shell"), compo::Transform{
		.translation = {0.156382, 0.777229, -0.161698},
		.scaling = {0.482906f},
	});
	hierarchy.attach(hierarchy.create("/hierarchy/shape/kernel"), compo::Transform{
		.translation = {0.110507, 0.494301, -0.126194},
		.scaling = {0.482906f},
	});
	hierarchy.attach(hierarchy.create("/hierarchy/shape/base"), compo::Transform{
		.translation = {0.0571719, 0.213656, -0.0682078},
		.scaling = {0.482906f},
	});
	hierarchy.attach(hierarchy.create("/hierarchy/shape/bound"), compo::Transform{
		.scaling = {500.0f},
	});
	hierarchy.attach(hierarchy.create("/hierarchy/medium"), compo::Transform{});
	hierarchy.attach(hierarchy.create("/hierarchy/medium/cloud"), compo::Transform{
		.rotation = math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 1.f / 2.f),
	});
	hierarchy.attach(hierarchy.create("/hierarchy/light"), compo::Transform{});
	hierarchy.attach(hierarchy.create("/hierarchy/light/environment"), compo::Transform{
		// .rotation = math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 1.f),
	});
	hierarchy.attach(hierarchy.create("/hierarchy/light/parallel"), compo::Transform{
		.rotation = math::Quaternion<f32>::from_rotation_between(
			{0.f, 0.f, 1.f},
			math::unit_sphere_to_cartesion({math::pi * 0.6f, math::pi * 0.3f})
		),
	});
	hierarchy.attach(hierarchy.create("/hierarchy/light/point"), compo::Transform{
		.translation = math::unit_sphere_to_cartesion({math::pi * 1.f / 4.f, math::pi * 4.f / 3.f}) * 1.5f,
	});
	hierarchy.attach(hierarchy.create("/hierarchy/light/spot"), compo::Transform{
		.translation = math::unit_sphere_to_cartesion({math::pi * 1.f / 4.f, math::pi * 4.f / 3.f}) * 1.5f,
		.rotation = math::Quaternion<f32>::from_rotation_between(
			{0.f, 0.f, 1.f},
			math::unit_sphere_to_cartesion({math::pi * 3.f / 4.f, math::pi * 1.f / 3.f})
		),
	});

	hierarchy.attach<compo::Shape>(hierarchy.create("/shape/sphere"), compo::Sphere{});
	hierarchy.attach(hierarchy.entity("/hierarchy/shape/bound"), compo::Shape_Instance{
		.path = hierarchy.entity("/shape/sphere")
	});
	hierarchy.attach<compo::Shape>(hierarchy.create("/shape/shell"), compo::Mesh{
		.path = "../metatron-scenes/material/mesh/shell.ply"
	});
	hierarchy.attach(hierarchy.entity("/hierarchy/shape/shell"), compo::Shape_Instance{
		.path = hierarchy.entity("/shape/shell")
	});
	hierarchy.attach<compo::Shape>(hierarchy.create("/shape/kernel"), compo::Mesh{
		.path = "../metatron-scenes/material/mesh/kernel.ply"
	});
	hierarchy.attach(hierarchy.entity("/hierarchy/shape/kernel"), compo::Shape_Instance{
		.path = hierarchy.entity("/shape/kernel")
	});
	hierarchy.attach<compo::Shape>(hierarchy.create("/shape/base"), compo::Mesh{
		.path = "../metatron-scenes/material/mesh/base.ply"
	});
	hierarchy.attach(hierarchy.entity("/hierarchy/shape/base"), compo::Shape_Instance{
		.path = hierarchy.entity("/shape/base")
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
	hierarchy.attach<compo::Spectrum>(hierarchy.create("/spectrum/diffuse-reflectance"), compo::Rgb_Spectrum{
		.c = {1.0f, 1.0f, 1.0f},
		.type = color::Color_Space::Spectrum_Type::albedo,
		.color_space = sRBB_entity,
	});
	hierarchy.attach<compo::Spectrum>(hierarchy.create("/spectrum/env"), compo::Rgb_Spectrum{
		.c = {0.03f, 0.07f, 0.23f},
		.type = color::Color_Space::Spectrum_Type::illuminant,
		.color_space = sRBB_entity
	});
	hierarchy.attach<compo::Spectrum>(hierarchy.create("/spectrum/parallel-illuminance"), compo::Rgb_Spectrum{
		.c = {2.6f, 2.5f, 2.3f},
		.type = color::Color_Space::Spectrum_Type::illuminant,
		.color_space = sRBB_entity,
	});
	hierarchy.attach<compo::Spectrum>(hierarchy.create("/spectrum/point-illuminance"), compo::Rgb_Spectrum{
		.c = {0.0f, 0.6f, 1.0f},
		.type = color::Color_Space::Spectrum_Type::illuminant,
		.color_space = sRBB_entity,
	});
	hierarchy.attach<compo::Spectrum>(hierarchy.create("/spectrum/spot-illuminance"), compo::Rgb_Spectrum{
		.c = {0.0f, 0.6f, 1.0f},
		.type = color::Color_Space::Spectrum_Type::illuminant,
		.color_space = sRBB_entity,
	});

	hierarchy.attach<compo::Texture>(hierarchy.create("/texture/diffuse"), compo::Constant_Spectrum_Texture{
		.spectrum = hierarchy.entity("/spectrum/diffuse-reflectance"),
	});
	hierarchy.attach<compo::Texture>(hierarchy.create("/texture/alpha"), compo::Constant_Vector_Texture{
		.x = {0.5f, 0.f, 0.f, 0.f},
	});
	hierarchy.attach<compo::Texture>(hierarchy.create("/texture/env-map"), compo::Image_Spectrum_Texture{
		.path = "../metatron-scenes/material/texture/sky-on-fire.exr",
		.type = color::Color_Space::Spectrum_Type::illuminant,
	});
	hierarchy.attach<compo::Texture>(hierarchy.create("/texture/const-env"), compo::Constant_Spectrum_Texture{
		.spectrum = hierarchy.entity("/spectrum/env"),
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

	auto lambertian = bsdf::Lambertian_Bsdf{};
	auto interface = bsdf::Interface_Bsdf{};
	auto microfacet = bsdf::Microfacet_Bsdf{};


	auto diffuse_material = material::Material{
		.bsdf = make_poly<bsdf::Bsdf, bsdf::Lambertian_Bsdf>(),
		.spectrum_textures = {
			{"reflectance", hierarchy.fetch<poly<texture::Texture<spectra::Stochastic_Spectrum>>>(hierarchy.entity("/texture/diffuse"))},
		},
	};
	auto test_material = material::Material{
		.bsdf = make_poly<bsdf::Bsdf, bsdf::Microfacet_Bsdf>(),
		.spectrum_textures = {
			{"eta", hierarchy.fetch<poly<texture::Texture<spectra::Stochastic_Spectrum>>>(hierarchy.entity("/texture/eta/Au"))},
			{"k", hierarchy.fetch<poly<texture::Texture<spectra::Stochastic_Spectrum>>>(hierarchy.entity("/texture/k/Au"))},
		},
		.vector_textures = {
			{"alpha", hierarchy.fetch<poly<texture::Texture<math::Vector<f32, 4>>>>(hierarchy.entity("/texture/alpha"))},
		},
	};
	auto interface_material = material::Material{
		.bsdf = make_poly<bsdf::Bsdf, bsdf::Interface_Bsdf>(),
	};

	auto env_light = light::Environment_Light{
		hierarchy.fetch<poly<texture::Texture<spectra::Stochastic_Spectrum>>>(hierarchy.entity("/texture/env-map"))
	};
	auto const_env_light = light::Environment_Light{
		hierarchy.fetch<poly<texture::Texture<spectra::Stochastic_Spectrum>>>(hierarchy.entity("/texture/const-env"))
	};
	auto parallel_light = light::Parallel_Light{
		hierarchy.fetch<poly<spectra::Spectrum>>(hierarchy.entity("/spectrum/parallel-illuminance")),
	};
	auto point_light = light::Point_Light{
		hierarchy.fetch<poly<spectra::Spectrum>>(hierarchy.entity("/spectrum/point-illuminance")),
	};
	auto spot_light = light::Spot_Light{
		hierarchy.fetch<poly<spectra::Spectrum>>(hierarchy.entity("/spectrum/spot-illuminance")),
		math::pi * 1.f / 16.f,
		math::pi * 1.f / 4.f
	};
	auto area_light = light::Area_Light{hierarchy.fetch<poly<shape::Shape>>(
		hierarchy.fetch<compo::Shape_Instance>(
			hierarchy.entity("/hierarchy/shape/bound")
		).path
	)};
	auto lights = std::vector<emitter::Divider>{
		// {&parallel_light, &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/light/parallel"))},
		// {&point_light, &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/light/parallel"))},
		// {&spot_light, &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/light/parallel"))},
		// {&area_light, &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/shape/sphere"))},
	};
	auto inf_lights = std::vector<emitter::Divider>{
		// {&const_env_light, &light_to_world},
		{&env_light, &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/light/environment"))},
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

	auto& shell = hierarchy.fetch<poly<shape::Shape>>(
		hierarchy.fetch<compo::Shape_Instance>(
			hierarchy.entity("/hierarchy/shape/shell")
		).path
	);
	auto& kernel = hierarchy.fetch<poly<shape::Shape>>(
		hierarchy.fetch<compo::Shape_Instance>(
			hierarchy.entity("/hierarchy/shape/kernel")
		).path
	);
	auto& base = hierarchy.fetch<poly<shape::Shape>>(
		hierarchy.fetch<compo::Shape_Instance>(
			hierarchy.entity("/hierarchy/shape/base")
		).path
	);

	// for (auto i = 0uz; i < shell->size(); i++) {
	// 	dividers.push_back({
	// 		.shape = shell,
	// 		.medium = &vaccum_medium,
	// 		.light = nullptr,
	// 		.material = &test_material,
	// 		.local_to_world = &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/shape/shell")),
	// 		.medium_to_world = &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/medium")),
	// 		.primitive = i,
	// 	});
	// }
	// for (auto i = 0uz; i < kernel->size(); i++) {
	// 	dividers.push_back({
	// 		.shape = kernel,
	// 		.medium = &vaccum_medium,
	// 		.light = nullptr,
	// 		.material = &diffuse_material,
	// 		.local_to_world = &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/shape/kernel")),
	// 		.medium_to_world = &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/medium")),
	// 		.primitive = i,
	// 	});
	// }
	// for (auto i = 0uz; i < base->size(); i++) {
	// 	dividers.push_back({
	// 		.shape = base,
	// 		.medium = &vaccum_medium,
	// 		.light = nullptr,
	// 		.material = &test_material,
	// 		.local_to_world = &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/shape/base")),
	// 		.medium_to_world = &hierarchy.fetch<math::Transform>(hierarchy.entity("/hierarchy/medium")),
	// 		.primitive = i,
	// 	});
	// }

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
