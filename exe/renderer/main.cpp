#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/sampler/independent.hpp>
#include <metatron/core/math/sampler/halton.hpp>
#include <metatron/core/math/filter/box.hpp>
#include <metatron/core/math/filter/gaussian.hpp>
#include <metatron/core/math/filter/lanczos.hpp>
#include <metatron/core/math/encode.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/spectra/constant.hpp>
#include <metatron/core/spectra/rgb.hpp>
#include <metatron/core/color/color-space.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/photo/lens/thin.hpp>
#include <metatron/render/light/environment.hpp>
#include <metatron/render/light/parallel.hpp>
#include <metatron/render/light/point.hpp>
#include <metatron/render/light/spot.hpp>
#include <metatron/render/light/area.hpp>
#include <metatron/render/emitter/uniform.hpp>
#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/render/accel/bvh.hpp>
#include <metatron/geometry/material/texture/image.hpp>
#include <metatron/geometry/material/texture/constant.hpp>
#include <metatron/geometry/material/diffuse.hpp>
#include <metatron/geometry/material/interface.hpp>
#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/volume/media/homogeneous.hpp>
#include <metatron/volume/media/vaccum.hpp>
#include <metatron/volume/media/grid.hpp>
#include <metatron/volume/media/nanovdb.hpp>
#include <metatron/volume/phase/henyey-greenstein.hpp>
#include <atomic>
#include <queue>
#include <thread>

using namespace metatron;

auto main() -> int {
	spectra::Spectrum::initialize();
	color::Color_Space::initialize();
	light::Light::initialize();
	material::Material::initialize();

	auto size = math::Vector<usize, 2>{600uz, 400uz};
	auto spp = 16uz;
	auto blocks = 8uz;
	auto depth = 100uz;
	auto kernels = usize(std::thread::hardware_concurrency());

	auto sensor = std::make_unique<photo::Sensor>(color::Color_Space::sRGB.get());
	auto lens = std::make_unique<photo::Thin_Lens>(5.6f, 0.05f, 10.f);
	auto film = std::make_unique<photo::Film>(
		math::Vector<f32, 2>{0.036f, 0.024f},
		size,
		std::move(sensor),
		std::make_unique<math::Lanczos_Filter>(),
		color::Color_Space::sRGB.get()
	);

	auto camera = photo::Camera{
		std::move(film),
		std::move(lens)
	};
	auto rd = std::random_device{};
	auto sampler = math::Halton_Sampler{rd()};

	auto identity = math::Transform{};
	auto world_to_render = math::Transform{{0.f, 0.f, 200.f}};
	auto render_to_camera = identity;

	auto sphere_to_world = math::Transform{{}, {1.f}};
	auto bound_to_world = math::Transform{{}, {100.f}};
	auto medium_to_world = math::Transform{{}, {0.2f},
		math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 1.f / 2.f),
	};
	auto light_to_world = identity;
	auto parallel_to_world = math::Transform{{}, {1.f},
		math::Quaternion<f32>::from_rotation_between(
			{0.f, 0.f, 1.f},
			math::sphere_to_cartesion({math::pi * 0.6f, math::pi * 1.3f})
		),
	};
	auto point_to_world = math::Transform{
		math::sphere_to_cartesion({math::pi * 1.f / 4.f, math::pi * 4.f / 3.f}) * 1.5f
	};
	auto spot_to_world = math::Transform{
		math::sphere_to_cartesion({math::pi * 1.f / 4.f, math::pi * 4.f / 3.f}) * 1.5f,
		{1.f},
		math::Quaternion<f32>::from_rotation_between(
			{0.f, 0.f, 1.f},
			math::sphere_to_cartesion({math::pi * 3.f / 4.f, math::pi * 1.f / 3.f})
		),
	};

	auto sphere = shape::Sphere{};
	auto diffuse_material = material::Diffuse_Material{
		std::make_unique<material::Constant_Texture<spectra::Stochastic_Spectrum>>(
			color::Color_Space::sRGB->to_spectrum(
				{1.0f, 1.0f, 1.0f},
				color::Color_Space::Spectrum_Type::albedo
			)
		),
		std::make_unique<material::Constant_Texture<spectra::Stochastic_Spectrum>>(
			color::Color_Space::sRGB->to_spectrum(
				{0.0f, 0.0f, 0.0f},
				color::Color_Space::Spectrum_Type::albedo
			)
		),
		std::make_unique<material::Constant_Texture<spectra::Stochastic_Spectrum>>(
			color::Color_Space::sRGB->to_spectrum(
				{0.0f, 0.6f, 1.0f},
				color::Color_Space::Spectrum_Type::illuminant
			)
		)
	};
	auto interface_material = material::Interface_Material{};

	auto vaccum_medium = media::Vaccum_Medium{};
	auto nanovdb_grid = media::Nanovdb_Grid<
		f32,
		media::grid_size,
		media::grid_size,
		media::grid_size
	>{
		"../Documents/metatron/disney-cloud.nvdb"
	};
	auto cloud_medium = media::Grid_Medium{
		&nanovdb_grid,
		color::Color_Space::sRGB->to_spectrum(
			{0.0f, 0.0f, 0.0f},
			color::Color_Space::Spectrum_Type::albedo
		),
		color::Color_Space::sRGB->to_spectrum(
			{1.0f, 1.0f, 1.0f},
			color::Color_Space::Spectrum_Type::albedo
		),
		color::Color_Space::sRGB->to_spectrum(
			{0.0f, 0.0f, 0.0f},
			color::Color_Space::Spectrum_Type::illuminant
		),
		std::make_unique<phase::Henyey_Greenstein_Phase_Function>(0.877f),
		4.f
	};

	auto env_map = std::make_unique<material::Image_Texture<spectra::Stochastic_Spectrum>>(
		image::Image::from_path("../Pictures/sky-on-fire.exr", true),
		color::Color_Space::Spectrum_Type::illuminant
	);
	auto env_light = light::Environment_Light{std::move(env_map)};
	auto const_env_light = light::Environment_Light{std::make_unique<material::Constant_Texture<spectra::Stochastic_Spectrum>>(
		color::Color_Space::sRGB->to_spectrum(
			{0.03f, 0.07f, 0.23f},
			color::Color_Space::Spectrum_Type::illuminant
		)
	)};
	auto parallel_light = light::Parallel_Light{
		color::Color_Space::sRGB->to_spectrum(
			{2.6f, 2.5f, 2.3f},
			color::Color_Space::Spectrum_Type::illuminant
		),
	};
	auto point_light = light::Point_Light{
		color::Color_Space::sRGB->to_spectrum(
			{0.0f, 0.6f, 1.0f},
			color::Color_Space::Spectrum_Type::illuminant
		)
	};
	auto spot_light = light::Spot_Light{
		color::Color_Space::sRGB->to_spectrum(
			{0.0f, 0.6f, 1.0f},
			color::Color_Space::Spectrum_Type::illuminant
		),
		math::pi * 1.f / 16.f,
		math::pi * 1.f / 4.f
	};
	auto area_light = light::Area_Light{sphere};
	auto lights = std::vector<emitter::Divider>{
		{&parallel_light, &parallel_to_world},
		// {&point_light, &point_to_world},
		// {&spot_light, &spot_to_world},
		// {&area_light, &sphere_to_world}
	};
	auto inf_lights = std::vector<emitter::Divider>{
		{&const_env_light, &light_to_world},
		// {&env_light, &light_to_world},
	};
	auto emitter = emitter::Uniform_Emitter{std::move(lights), std::move(inf_lights)};

	auto bvh = accel::LBVH{{
		{
			&sphere,
			&cloud_medium,
			&vaccum_medium,
			&interface_material,
			nullptr,
			&bound_to_world,
			&medium_to_world,
			&identity,
			0uz
		},
		// {
		// 	&sphere,
		// 	&vaccum_medium,
		// 	&vaccum_medium,
		// 	&diffuse_material,
		// 	&area_light,
		// 	&sphere_to_world,
		// 	&identity,
		// 	&identity,
		// 	0uz
		// },
	}};

	auto integrator = mc::Volume_Path_Integrator{};

	auto block_queue = std::vector<std::queue<math::Vector<usize, 2>>>(kernels);
	auto w_blocks = (size[0] + blocks - 1) / blocks;
	auto h_blocks = (size[1] + blocks - 1) / blocks;
	for (auto i = 0uz; i < w_blocks; i++) {
		for (auto j = 0uz; j < h_blocks; j++) {
			auto m = math::morton_encode(i, j);
			block_queue[m % kernels].push({i, j});
		}
	}

	auto atomic_count = std::atomic<usize>{0uz};
	auto dispatch = [&](usize idx) -> void {
		while (!block_queue[idx].empty()) {
			auto block_idx = block_queue[idx].front();
			block_queue[idx].pop();

			auto start = blocks * block_idx;
			for (auto p = 0; p < blocks * blocks; p++) {
				auto px = start + math::morton_decode(p);
				if (px >= size) {
					continue;
				}

				for (auto n = 0uz; n < spp; n++) {
					auto sample = camera.sample(px, n, sampler);
					sample->ray_differential = render_to_camera ^ sample->ray_differential;
					auto& s = sample.value();

					auto Li_opt = integrator.sample(
						{
							s.ray_differential,
							s.default_differential,
							&vaccum_medium,
							&world_to_render,
							&render_to_camera,
							&identity,
							depth
						},
						bvh,
						emitter,
						sampler
					);

					auto& Li = Li_opt.value();
					s.fixel = Li;
					atomic_count.fetch_add(1);
				}
			}
		}
	};

	auto threads = std::vector<std::thread>{};
	for (auto i = 0uz; i < kernels; i++) {
		threads.emplace_back(dispatch, i);
	}

	while (true) {
		auto total = size[0] * size[1] * spp;
		auto count = atomic_count.load();
		if (count < total) {
			std::printf("\r");
			std::printf("%f", 1.f * count / total);
		} else {
			for (auto i = 0uz; i < kernels; i++) {
				threads[i].join();
			}
			break;
		}
	}

	camera.to_path("build/test.exr");
	return 0;
}
