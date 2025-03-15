#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/sampler/independent.hpp>
#include <metatron/core/math/filter/box.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/spectra/rgb.hpp>
#include <metatron/core/spectra/test-rgb.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/divider/bvh.hpp>
#include <metatron/render/material/texture/spectrum.hpp>
#include <metatron/render/material/bsdf/lambertian.hpp>
#include <metatron/render/light/environment.hpp>
#include <metatron/render/light/test.hpp>
#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/volume/media/homogeneous.hpp>

using namespace metatron;

auto main() -> int {
	auto sensor = std::make_unique<photo::Sensor>(
		std::make_unique<spectra::Test_Rgb_Spectrum>(300.f, 450.f),
		std::make_unique<spectra::Test_Rgb_Spectrum>(450.f, 600.f),
		std::make_unique<spectra::Test_Rgb_Spectrum>(600.f, 800.f)
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
	auto stochastic = spectra::Stochastic_Spectrum{3uz, 0.f};

	auto sphere = shape::Sphere{1.f, 0.f, math::pi, 2.f * math::pi};
	auto homo_medium = media::Homogeneous_Medium{
		std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.1f, 0.2f, 0.3f}),
		std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.1f, 0.2f, 0.3f})
	};
	auto lambertian = material::Lambertian_Bsdf{
		std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.5f, 0.6f, 0.7f}),
		std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.0f})
	};
	auto divider = divider::Divider{&sphere, &homo_medium, &lambertian};
	auto bvh = divider::LBVH{{&divider}};

	auto env_map = image::Image::from_path("/home/tsssni/Downloads/the_sky_is_on_fire_4k.exr");
	auto env_tex = std::make_unique<material::Spectrum_Image_Texture>(std::move(env_map));
	auto env_light = light::Environment_Light{std::move(env_tex)};
	auto lights = std::vector<light::Light const*>{&env_light};
	auto inf_lights = std::vector<light::Light const*>{&env_light};
	auto emitter = light::Test_Emitter{std::move(lights), std::move(inf_lights)};

	for (auto j = 0uz; j < 1024uz; j++) {
		for (auto i = 0uz; i < 1024uz; i++) {
			for (auto n = 0uz; n < 1; n++) {
				auto sample = camera.sample(math::Vector<usize, 2>{i, j}, 0, sampler);
				auto& s = sample.value();
				s.r.o[2] -= 3.f;
				auto intr = bvh(s.r);

				if (intr) {
					auto& intrv = intr.value();
					auto rot = math::Matrix<f32, 4, 4>{math::Quaternion<f32>::from_rotation_between(intrv.intr.n, {0.f, 1.f, 0.f})};
					auto inv_rot = math::inverse(rot);

					auto wo = math::normalize(rot * math::Vector<f32, 4>{s.r.d});
					auto intr = intrv.divider->bsdf->sample({&stochastic, wo}, {sampler.generate_1d(), sampler.generate_1d(), sampler.generate_1d()}).value();

					auto rr = math::Ray{intrv.intr.p, inv_rot * math::Vector<f32, 4>{intr.wi}};
					auto env = emitter(rr);
					s.fixel = *intr.f & *env.value();
				} else {
					auto env = emitter(s.r);
					auto L = stochastic & *env.value();
					s.fixel = L;
				}
			}
		}
	}
	camera.to_path("build/test.exr");
	return 0;
}
