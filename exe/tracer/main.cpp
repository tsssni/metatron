#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/scene/serde/serde.hpp>
#include <metatron/scene/compo/spectrum.hpp>
#include <metatron/scene/compo/transform.hpp>
#include <metatron/scene/compo/shape.hpp>
#include <metatron/scene/compo/medium.hpp>
#include <metatron/scene/compo/texture.hpp>
#include <metatron/scene/compo/material.hpp>
#include <metatron/scene/compo/light.hpp>
#include <metatron/scene/compo/camera.hpp>
#include <metatron/scene/compo/tracer.hpp>
#include <metatron/scene/daemon/spectrum.hpp>
#include <metatron/scene/daemon/color-space.hpp>
#include <metatron/scene/daemon/transform.hpp>
#include <metatron/scene/daemon/shape.hpp>
#include <metatron/scene/daemon/medium.hpp>
#include <metatron/scene/daemon/light.hpp>
#include <metatron/scene/daemon/texture.hpp>
#include <metatron/scene/daemon/material.hpp>
#include <metatron/scene/daemon/camera.hpp>
#include <metatron/scene/daemon/tracer.hpp>

using namespace mtt;

auto main() -> int {
	auto hierarchy = ecs::Hierarchy{};

	auto spectrum_stage = make_poly<ecs::Stage>();
	auto resource_stage = make_poly<ecs::Stage>();
	auto material_stage = make_poly<ecs::Stage>();
	auto camera_stage = make_poly<ecs::Stage>();
	auto render_stage = make_poly<ecs::Stage>();

	auto spectrum_daemon = daemon::Spectrum_Daemon{};
	auto color_space_daemon = daemon::Color_Space_Daemon{};
	auto transform_daemon = daemon::Transform_Daemon{};
	auto shape_daemon = daemon::Shape_Daemon{};
	auto medium_daemon = daemon::Medium_Daemon{};
	auto texture_daemon = daemon::Texture_Daemon{};
	auto light_daemon = daemon::Light_Daemon{};
	auto material_daemon = daemon::Material_Daemon{};
	auto camera_daemon = daemon::Camera_Daemon{};
	auto tracer_daemon = daemon::Tracer_Daemon{};

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
		&camera_daemon,
	};
	camera_stage->daemons = {
		&camera_daemon,
	};
	render_stage->daemons = {
		&tracer_daemon,
	};
	hierarchy.stages = {
		spectrum_stage.get(),
		resource_stage.get(),
		material_stage.get(),
		camera_stage.get(),
		render_stage.get(),
	};
	hierarchy.activate();
	hierarchy.init();

	auto sRGB_entity = "/color-space/sRGB"_et;
	auto* sRGB = &hierarchy.fetch<color::Color_Space>(sRGB_entity);

	hierarchy.attach<compo::Spectrum>("/spectrum/sigma-a"_et, compo::Rgb_Spectrum{
		.c = {0.0f, 0.0f, 0.0f},
		.type = color::Color_Space::Spectrum_Type::unbounded,
		.color_space = sRGB_entity,
	});
	hierarchy.attach<compo::Spectrum>("/spectrum/sigma-s"_et, compo::Rgb_Spectrum{
		.c = {1.0f, 1.0f, 1.0f},
		.type = color::Color_Space::Spectrum_Type::unbounded,
		.color_space = sRGB_entity,
	});
	hierarchy.attach<compo::Spectrum>("/spectrum/sigma-e"_et, compo::Rgb_Spectrum{
		.c = {0.0f, 0.0f, 0.0f},
		.type = color::Color_Space::Spectrum_Type::illuminant,
		.color_space = sRGB_entity,
	});
	hierarchy.attach<compo::Spectrum>("/spectrum/env"_et, compo::Rgb_Spectrum{
		.c = {0.03f, 0.07f, 0.23f},
		.type = color::Color_Space::Spectrum_Type::illuminant,
		.color_space = sRGB_entity
	});

	hierarchy.attach("/hierarchy/camera"_et,compo::Transform{
		.translation = {0.f, 0.f, -1000.f},
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

	hierarchy.attach<compo::Medium>("/medium/cloud"_et, compo::Grid_Medium{
		.path = "../metatron-scenes/disney-cloud/volume/disney-cloud.nvdb",
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

	hierarchy.attach("/hierarchy/camera"_et, compo::Camera{
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
		.initial_medium = "/hierarchy/medium/vaccum"_et,
		.color_space = sRGB_entity,
	});

	hierarchy.attach("/divider/cloud"_et, compo::Divider{
		.shape = "/hierarchy/shape/bound"_et,
		.medium = "/hierarchy/medium/cloud"_et,
		.material = "/material/cloud"_et,
	});
	hierarchy.attach("/tracer"_et, compo::Tracer{
		.emitter = compo::Uniform_Emitter{},
		.accel = compo::LBVH{},
		.integrator = compo::Volume_Path_Integrator{},
	});

	hierarchy.update();
	hierarchy.write("build/test.json");
	tracer_daemon.render("build/test.exr");
	return 0;
}
