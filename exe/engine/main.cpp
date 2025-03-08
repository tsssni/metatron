#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sampler/independent.hpp>
#include <metatron/core/math/filter/box.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/spectra/stochastic.hpp>
#include <metatron/render/spectra/rgb.hpp>
#include <metatron/render/spectra/test-rgb.hpp>
#include <metatron/render/divider/bvh.hpp>
#include <metatron/geometry/shape/sphere.hpp>

using namespace metatron;

auto main() -> int {
	auto sensor = std::make_unique<photo::Sensor>(
		std::make_unique<spectra::Test_Rgb_Spectrum>(380.f, 380.f + 400.f / 3.f),
		std::make_unique<spectra::Test_Rgb_Spectrum>(380.f + 400.f / 3.f, 380.f + 400.f * 2.f / 3.f),
		std::make_unique<spectra::Test_Rgb_Spectrum>(380.f + 400.f * 2.f / 3.f, 780.f)
	);
	auto lens = std::make_unique<photo::Pinhole_Lens>(0.1f);
	auto film = std::make_unique<photo::Film>(
		math::Vector<f32, 2>{0.1f, 0.1f},
		math::Vector<usize, 2>{1024, 1024},
		std::move(sensor),
		std::make_unique<math::Box_Filter>()
	);

	auto camera = photo::Camera{
		std::move(film),
		std::move(lens)
	};
	auto sampler = math::Independent_Sampler{};
	auto spectrum = spectra::Rgb_Spectrum{{0.5f, 0.6f, 0.7f}};

	auto sphere = shape::Sphere{0.1f, 0.f, math::pi, 2.f * math::pi};
	auto divider = divider::Divider{&sphere};
	auto bvh = divider::LBVH{{&divider}};

	for (auto j = 0uz; j < 1024uz; j++) {
		for (auto i = 0uz; i < 1024uz; i++) {
			auto sample = camera.sample(math::Vector<usize, 2>{i, j}, 0, sampler);
			sample.r.o[2] -= 1.f;
			auto intr = bvh.intersect(sample.r);
			if (intr) sample.fixel = spectrum;
		}
	}
	camera.to_path("build/test.exr");
	return 0;
}
