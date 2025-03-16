#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/sampler/independent.hpp>
#include <metatron/core/math/filter/box.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/spectra/rgb.hpp>
#include <metatron/core/spectra/test-rgb.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/light/environment.hpp>
#include <metatron/render/light/test.hpp>
#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/geometry/divider/bvh.hpp>
#include <metatron/geometry/material/texture/spectrum/image.hpp>
#include <metatron/geometry/material/texture/spectrum/constant.hpp>
#include <metatron/geometry/material/diffuse.hpp>
#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/volume/media/homogeneous.hpp>
#include <metatron/volume/phase/henyey-greenstein.hpp>
#include <metatron/hierarchy/transform.hpp>

using namespace metatron;

auto main() -> int {
	auto constexpr size = math::Vector<usize, 2>{128uz};
	auto constexpr spp = 128uz;

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
	auto transform = math::Matrix<f32, 4, 4>{hierarchy::Transform{{0.f, 0.f, -3.f}}};

	auto sphere = shape::Sphere{1.f, 0.f, math::pi, 2.f * math::pi};
	auto diffuse = material::Diffuse_Material{
		std::make_unique<material::Spectrum_Constant_Texture>(
			std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.1f})
		),
		std::make_unique<material::Spectrum_Constant_Texture>(
			std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.0f})
		),
	};
	auto homo_medium = media::Homogeneous_Medium{
		std::make_unique<phase::Henyey_Greenstein_Phase_Function>(0.5f),
		std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.1f}),
		std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.8f}),
		std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.1f})
	};
	auto divider = divider::Divider{&sphere, &diffuse, &homo_medium};
	auto bvh = divider::LBVH{{&divider}};

	auto env_map = image::Image::from_path("/home/tsssni/Downloads/the_sky_is_on_fire_4k.exr");
	auto env_light = light::Environment_Light{std::move(env_map)};
	auto lights = std::vector<light::Light const*>{&env_light};
	auto inf_lights = std::vector<light::Light const*>{&env_light};
	auto emitter = light::Test_Emitter{std::move(lights), std::move(inf_lights)};
	auto integrator = mc::Volume_Path_Integrator{};

	for (auto j = 0uz; j < size[1]; j++) {
		for (auto i = 0uz; i < size[0]; i++) {
			for (auto n = 0uz; n < spp; n++) {
				auto sample = camera.sample(math::Vector<usize, 2>{i, j}, n, sampler);
				auto& s = sample.value();
				s.r.o = transform | math::Vector<f32, 4>{s.r.o, 1.f};
				s.r.d = transform | math::Vector<f32, 4>{s.r.d};

				auto Li_opt = integrator.sample({s.r}, bvh, emitter, sampler);
				auto& Li = Li_opt.value();
				s.fixel = *Li;
			}

			std::printf("\r");
			std::printf("%f", 1.f * (j * 128uz + i) / (128uz * 128uz));
		}
	}
	camera.to_path("build/test.exr");
	return 0;
}
