#include <metatron/core/math/quaternion.hpp>
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

	auto size = math::Vector<usize, 2>{600uz, 400uz};
	auto spp = 1024uz;
	auto blocks = 8uz;
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
	auto world_to_render = math::Transform{{0.f, 0.f, 500.f}};
	auto render_to_camera = identity;

	auto sphere_to_world = math::Transform{{}, {250.f}};
	auto medium_to_world = math::Transform{{}, {0.5f},
		math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi / 2.f),
	};
	auto light_to_world = math::Transform{{}, {1.f},
		math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi / 1.f),
	};

	auto sphere = shape::Sphere{};
	auto diffuse_material = material::Diffuse_Material{
		std::make_unique<material::Constant_Texture<spectra::Stochastic_Spectrum>>(
			std::make_unique<spectra::Constant_Spectrum>(0.5f)
		),
		std::make_unique<material::Constant_Texture<spectra::Stochastic_Spectrum>>(
			std::make_unique<spectra::Constant_Spectrum>(0.0f)
		),
	};
	auto interface_material = material::Interface_Material{};

	auto nanovdb_grid = media::Nanovdb_Grid<
		f32,
		media::grid_size,
		media::grid_size,
		media::grid_size
	>{
		"../Documents/metatron/disney-cloud.nvdb"
	};
	auto vaccum_medium = media::Vaccum_Medium{};
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
		std::make_unique<phase::Henyey_Greenstein_Phase_Function>(0.0f),
	};
	auto bvh = accel::LBVH{{
		{
			&sphere,
			&cloud_medium,
			&vaccum_medium,
			&interface_material,
			nullptr,
			&sphere_to_world,
			&medium_to_world,
			&identity,
			0uz
		}
	}};

	auto env_map = std::make_unique<material::Image_Texture<spectra::Stochastic_Spectrum>>(
		image::Image::from_path("../Pictures/sky-on-fire.exr", true),
		color::Color_Space::Spectrum_Type::illuminant
	);
	auto env_light = light::Environment_Light{std::move(env_map)};
	auto lights = std::vector<emitter::Divider>{{&light_to_world, &env_light}};
	auto inf_lights = std::vector<emitter::Divider>{{&light_to_world, &env_light}};
	auto emitter = emitter::Uniform_Emitter{std::move(lights), std::move(inf_lights)};

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
							&identity
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
