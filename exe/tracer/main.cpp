#include <metatron/scene/ecs/hierarchy.hpp>
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
#include <argparse/argparse.hpp>

using namespace mtt;

auto main(int argc, char* argv[]) -> int {
	auto argparser = argparse::ArgumentParser{"metatron-tracer", "0.0.1"};

	argparser.add_argument("-s", "--scene")
	.required()
	.help("directory contains scene.json")
	.default_value(std::string{"./"});

	argparser.add_argument("-o", "--output")
	.required()
	.help("result image path in exr format")
	.default_value(std::string{"./result.exr"});

	try {
		argparser.parse_args(argc, argv);
	}
	catch (const std::exception& err) {
		std::println("argparser error: {}", err.what());
		std::cout << argparser;
		return 1;
	}

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
	hierarchy.read(argparser.get<std::string>("-s"));
	hierarchy.update();
	tracer_daemon.render(argparser.get<std::string>("-o"));
	return 0;
}
