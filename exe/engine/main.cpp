#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/sampler/independent.hpp>
#include <metatron/core/math/filter/box.hpp>
#include <metatron/core/math/encode.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/spectra/rgb.hpp>
#include <metatron/core/spectra/test-rgb.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/light/environment.hpp>
#include <metatron/render/emitter/uniform.hpp>
#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/render/accel/bvh.hpp>
#include <metatron/geometry/material/texture/spectrum/image.hpp>
#include <metatron/geometry/material/texture/spectrum/constant.hpp>
#include <metatron/geometry/material/diffuse.hpp>
#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/volume/media/homogeneous.hpp>
#include <metatron/volume/phase/henyey-greenstein.hpp>
#include <metatron/hierarchy/transform.hpp>
#include <atomic>
#include <queue>
#include <thread>

using namespace metatron;

auto main() -> int {
	auto constexpr size = math::Vector<usize, 2>{256uz};
	auto constexpr spp = 16uz;
	auto constexpr blocks = 8uz;
	auto constexpr kernels = 12uz;

	auto sensor = std::make_unique<photo::Sensor>(
		std::make_unique<spectra::Test_Rgb_Spectrum>(300.f, 450.f),
		std::make_unique<spectra::Test_Rgb_Spectrum>(450.f, 600.f),
		std::make_unique<spectra::Test_Rgb_Spectrum>(600.f, 800.f)
	);
	auto lens = std::make_unique<photo::Pinhole_Lens>(0.1f);
	auto film = std::make_unique<photo::Film>(
		math::Vector<f32, 2>{0.1f, 0.1f},
		size,
		std::move(sensor),
		std::make_unique<math::Box_Filter>()
	);

	auto camera = photo::Camera{
		std::move(film),
		std::move(lens)
	};
	auto sampler = math::Independent_Sampler{};
	auto stochastic = spectra::Stochastic_Spectrum{3uz, 0.f};
	auto identity = hierarchy::Transform{};
	auto transform = math::Matrix<f32, 4, 4>{hierarchy::Transform{{0.f, 0.f, -3.f}}};

	auto sphere = shape::Sphere{1.f, 0.f, math::pi, 2.f * math::pi};
	auto diffuse = material::Diffuse_Material{
		std::make_unique<material::Spectrum_Constant_Texture>(
			std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.5f})
		),
		std::make_unique<material::Spectrum_Constant_Texture>(
			std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.0f})
		),
	};
	auto homo_medium = media::Homogeneous_Medium{
		std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.1f}),
		std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.5f}),
		std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.01f})
	};
	auto bvh = accel::LBVH{{accel::Divider{&identity, &sphere, 0uz, &diffuse, &homo_medium}}};

	auto env_map = image::Image::from_path("/home/tsssni/Downloads/the_sky_is_on_fire_4k.exr");
	auto env_light = light::Environment_Light{std::move(env_map)};
	auto lights = std::vector<emitter::Divider>{{&identity, &env_light}};
	auto inf_lights = std::vector<emitter::Divider>{{&identity, &env_light}};
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
				if (px >= size) continue;

				for (auto n = 0uz; n < spp; n++) {
					auto sample = camera.sample(px, n, sampler);
					auto& s = sample.value();
					s.r.o = transform | math::Vector<f32, 4>{s.r.o, 1.f};
					s.r.d = transform | math::Vector<f32, 4>{s.r.d};

					auto Li_opt = integrator.sample({s.r}, bvh, emitter, sampler);
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
		auto constexpr total = size[0] * size[1] * spp;
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
