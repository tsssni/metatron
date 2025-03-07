#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sampler/independent.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/divider/bvh.hpp>
#include <metatron/geometry/shape/sphere.hpp>

using namespace metatron;

namespace metatron {
	namespace spectra {
		struct Test_Spectrum final: Spectrum {
			f32 data = 1.f;

			auto operator()(f32 lambda) -> f32& {
				return data;
			}

			auto operator()(f32 lambda) const -> f32 const& {
				return data;
			}

			auto operator*(Spectrum const& spectrum) const -> f32 {
				return data;
			}
		};
	}
}

auto main() -> int {
	auto sensor = std::make_unique<photo::Sensor>(
		std::make_unique<spectra::Test_Spectrum>(),
		std::make_unique<spectra::Test_Spectrum>(),
		std::make_unique<spectra::Test_Spectrum>()
	);
	auto lens = std::make_unique<photo::Pinhole_Lens>(0.1f);
	auto film = std::make_unique<photo::Film>(
		math::Vector<f32, 2>{0.1f, 0.1f},
		math::Vector<usize, 2>{1024, 1024},
		std::move(sensor),
		std::make_unique<math::Filter>()
	);

	auto camera = photo::Camera{
		std::move(film),
		std::move(lens)
	};
	auto sampler = math::Independent_Sampler{};
	auto spectrum = spectra::Stochastic_Spectrum{4, 0.f};

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
