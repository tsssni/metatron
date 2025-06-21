#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/complex.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/sampler/independent.hpp>
#include <metatron/core/math/sampler/halton.hpp>
#include <metatron/core/math/filter/box.hpp>
#include <metatron/core/math/filter/gaussian.hpp>
#include <metatron/core/math/filter/lanczos.hpp>
#include <metatron/core/math/encode.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/lambertian.hpp>
#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/resource/bsdf/microfacet.hpp>
#include <metatron/resource/shape/sphere.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/loader/assimp.hpp>
#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/resource/media/vaccum.hpp>
#include <metatron/resource/media/grid.hpp>
#include <metatron/resource/media/nanovdb.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/lens/pinhole.hpp>
#include <metatron/render/lens/thin.hpp>
#include <metatron/render/light/environment.hpp>
#include <metatron/render/light/parallel.hpp>
#include <metatron/render/light/point.hpp>
#include <metatron/render/light/spot.hpp>
#include <metatron/render/light/area.hpp>
#include <metatron/render/emitter/uniform.hpp>
#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/render/accel/lbvh.hpp>
#include <atomic>

using namespace metatron;

auto main() -> int {
	spectra::Spectrum::initialize();
	color::Color_Space::initialize();

	auto size = math::Vector<usize, 2>{600uz, 400uz};
	auto spp = 64uz;
	auto depth = 128uz;

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
	auto world_to_render = math::Transform{{-5.f, -0.5f, 5.f}};
	// auto world_to_render = math::Transform{{0.f, 0.f, 1000.f}};
	auto render_to_camera = math::Transform{{0.f, 0.f, 0.f}, {1.f},
		math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 1.f / 4.f),
	};

	auto sphere_to_world = math::Transform{{}, {200.f}};
	auto mesh_to_world = math::Transform{{}, {100.f},
		math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 1.f / 1.f),
	};
	auto shell_to_world = math::Transform{{0.156382, 0.777229, -0.161698}, {0.482906f},
		math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 0.f / 1.f),
	};
	auto kernel_to_world = math::Transform{{0.110507, 0.494301, -0.126194}, {0.482906f},
		math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 0.f / 1.f),
	};
	auto base_to_world = math::Transform{{0.0571719, 0.213656, -0.0682078}, {0.482906f},
		math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 0.f / 1.f),
	};
	auto bound_to_world = math::Transform{{}, {500.f}};
	auto medium_to_world = math::Transform{{}, {1.0f},
		math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 1.f / 2.f),
	};
	auto light_to_world = math::Transform{{}, {1.0f},
		math::Quaternion<f32>::from_axis_angle({0.f, 1.f, 0.f}, math::pi * 1.f / 1.f),
	};
	auto parallel_to_world = math::Transform{{}, {1.f},
		math::Quaternion<f32>::from_rotation_between(
			{0.f, 0.f, 1.f},
			math::unit_sphere_to_cartesion({math::pi * 0.6f, math::pi * 0.3f})
		),
	};
	auto point_to_world = math::Transform{
		math::unit_sphere_to_cartesion({math::pi * 1.f / 4.f, math::pi * 4.f / 3.f}) * 1.5f
	};
	auto spot_to_world = math::Transform{
		math::unit_sphere_to_cartesion({math::pi * 1.f / 4.f, math::pi * 4.f / 3.f}) * 1.5f,
		{1.f},
		math::Quaternion<f32>::from_rotation_between(
			{0.f, 0.f, 1.f},
			math::unit_sphere_to_cartesion({math::pi * 3.f / 4.f, math::pi * 1.f / 3.f})
		),
	};

	auto sphere = shape::Sphere{};
	auto triangle = shape::Mesh{
		{{0, 1, 2}},
		{{-1, -1, 0}, {0, 1, 0}, {1, -1, 0}},
		{{0, 0, -1}, {0, 0, -1}, {0, 0, -1}},
		{{1, 0}, {0, 0}, {0, 1}},
	};

	auto vaccum_medium = media::Vaccum_Medium{};
	auto sigma_a = color::Color_Space::sRGB->to_spectrum(
		{0.0f, 0.0f, 0.0f},
		color::Color_Space::Spectrum_Type::unbounded
	);
	auto sigma_s = color::Color_Space::sRGB->to_spectrum(
		{1.0f, 1.0f, 1.0f},
		color::Color_Space::Spectrum_Type::unbounded
	);
	auto sigma_e = color::Color_Space::sRGB->to_spectrum(
		{0.0f, 0.0f, 0.0f},
		color::Color_Space::Spectrum_Type::illuminant
	);
	auto hg_phase = phase::Henyey_Greenstein_Phase_Function{0.877f};
	auto nanovdb_grid = media::Nanovdb_Grid<
		f32,
		media::grid_size,
		media::grid_size,
		media::grid_size
	>{
		"../metatron-assets/disney-cloud/volume/disney-cloud.nvdb"
	};
	auto cloud_medium = media::Grid_Medium{
		&nanovdb_grid,
		&hg_phase,
		sigma_a.get(),
		sigma_s.get(),
		sigma_e.get(),
		1.f,
	};

	auto lambertian = bsdf::Lambertian_Bsdf{};
	auto interface = bsdf::Interface_Bsdf{};
	auto microfacet = bsdf::Microfacet_Bsdf{};

	auto diffuse_reflectance = texture::Constant_Texture<spectra::Stochastic_Spectrum>{
		color::Color_Space::sRGB->to_spectrum(
			{1.f, 1.f, 1.f},
			color::Color_Space::Spectrum_Type::albedo
		)
	};
	auto diffuse_transmittance = texture::Constant_Texture<spectra::Stochastic_Spectrum>{
		color::Color_Space::sRGB->to_spectrum(
			{0.f, 0.f, 0.f},
			color::Color_Space::Spectrum_Type::albedo
		)
	};
	auto test_reflectance = texture::Constant_Texture<spectra::Stochastic_Spectrum>{
		color::Color_Space::sRGB->to_spectrum(
			{1.0f, 1.0f, 1.0f},
			color::Color_Space::Spectrum_Type::albedo
		)
	};
	auto test_transmittance = texture::Constant_Texture<spectra::Stochastic_Spectrum>{
		color::Color_Space::sRGB->to_spectrum(
			{0.0f, 0.0f, 0.0f},
			color::Color_Space::Spectrum_Type::albedo
		)
	};
	auto emission = texture::Constant_Texture<spectra::Stochastic_Spectrum>{
		color::Color_Space::sRGB->to_spectrum(
			{0.f, 0.f, 0.f},
			color::Color_Space::Spectrum_Type::illuminant
		)
	};
	auto normal = texture::Constant_Texture<math::Vector<f32, 4>>{
		math::Vector<f32, 4>{0.f, 0.f, 1.f, 0.f}
	};
	auto eta = spectra::Constant_Spectrum{1.0f};
	auto test_eta = spectra::Constant_Spectrum{1.5f};
	auto k = spectra::Constant_Spectrum{0.f};
	auto u_roughness = texture::Constant_Texture<math::Vector<f32, 4>>{
		math::Vector<f32, 4>{0.1f, 0.f, 0.f, 0.f}
	};
	auto v_roughness = texture::Constant_Texture<math::Vector<f32, 4>>{
		math::Vector<f32, 4>{0.1f, 0.f, 0.f, 0.f}
	};
	
	auto diffuse_material = material::Material{
		.bsdf = &lambertian,
		.interior_medium = &vaccum_medium,
		.exterior_medium = &vaccum_medium,
		.interior_eta = &eta,
		.exterior_eta = &eta,
		.interior_k = &k,
		.exterior_k = &k,
		.reflectance = &diffuse_reflectance,
		.transmittance = &diffuse_transmittance,
	};
	auto test_material = material::Material{
		.bsdf = &microfacet,
		.interior_medium = &vaccum_medium,
		.exterior_medium = &vaccum_medium,
		.interior_eta = &test_eta,
		.exterior_eta = &eta,
		.interior_k = &k,
		.exterior_k = &k,
		.u_roughness = &u_roughness,
		.v_roughness = &v_roughness,
	};
	auto interface_material = material::Material{
		.bsdf = &interface,
		.interior_medium = &cloud_medium,
		.exterior_medium = &vaccum_medium,
	};

	auto env_map = std::make_unique<texture::Image_Texture<spectra::Stochastic_Spectrum>>(
		image::Image::from_path("../metatron-assets/material/texture/sky-on-fire.exr", true),
		color::Color_Space::Spectrum_Type::illuminant
	);
	auto env_light = light::Environment_Light{std::move(env_map)};
	auto const_env_light = light::Environment_Light{std::make_unique<texture::Constant_Texture<spectra::Stochastic_Spectrum>>(
		color::Color_Space::sRGB->to_spectrum(
			// {0.03f, 0.07f, 0.23f},
			{1.f, 1.f, 1.f},
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
		// {&parallel_light, &parallel_to_world},
		// {&point_light, &point_to_world},
		// {&spot_light, &spot_to_world},
		// {&area_light, &sphere_to_world}
	};
	auto inf_lights = std::vector<emitter::Divider>{
		// {&const_env_light, &light_to_world},
		{&env_light, &light_to_world},
	};
	auto emitter = emitter::Uniform_Emitter{std::move(lights), std::move(inf_lights)};

	auto dividers = std::vector<accel::Divider>{
		// {
		// 	.shape = &sphere,
		// 	.material = &interface_material,
		// 	.light = nullptr,
		// 	.local_to_world = &bound_to_world,
		// 	.interior_to_world = &medium_to_world,
		// 	.exterior_to_world = &identity,
		// },
	};

	auto assimp_loader = loader::Assimp_Loader{};
	auto shell = assimp_loader.from_path("../metatron-assets/material/mesh/shell.ply");
	auto kernel = assimp_loader.from_path("../metatron-assets/material/mesh/kernel.ply");
	auto base = assimp_loader.from_path("../metatron-assets/material/mesh/base.ply");

	for (auto& mesh: shell) {
		for (auto i = 0uz; i < mesh->size(); i++) {
			dividers.push_back({
				.shape = mesh.get(),
				.material = &test_material,
				.light = nullptr,
				.local_to_world = &shell_to_world,
				.interior_to_world = &identity,
				.exterior_to_world = &identity,
				.primitive = i,
			});
		}
	}
	for (auto& mesh: kernel) {
		for (auto i = 0uz; i < mesh->size(); i++) {
			dividers.push_back({
				.shape = mesh.get(),
				.material = &diffuse_material,
				.light = nullptr,
				.local_to_world = &kernel_to_world,
				.interior_to_world = &identity,
				.exterior_to_world = &identity,
				.primitive = i,
			});
		}
	}
	for (auto& mesh: base) {
		for (auto i = 0uz; i < mesh->size(); i++) {
			dividers.push_back({
				.shape = mesh.get(),
				.material = &test_material,
				.light = nullptr,
				.local_to_world = &base_to_world,
				.interior_to_world = &identity,
				.exterior_to_world = &identity,
				.primitive = i,
			});
		}
	}
	auto bvh = accel::LBVH{std::move(dividers), &world_to_render};
	auto integrator = monte_carlo::Volume_Path_Integrator{};

	auto atomic_count = std::atomic<usize>{0uz};
	auto future = stl::Dispatcher::instance().async_parallel(size, [&](math::Vector<usize, 2> const& px) {
		for (auto n = 0uz; n < spp; n++) {
			auto sample = camera.sample(px, n, sampler);
			sample->ray_differential = render_to_camera ^ sample->ray_differential;
			auto& s = sample.value();

			auto Li_opt = integrator.sample(
				{
					s.ray_differential,
					s.default_differential,
					&world_to_render,
					&render_to_camera,
					depth,
				},
				bvh,
				emitter,
				sampler
			);

			auto& Li = Li_opt.value();
			s.fixel = Li;
			atomic_count.fetch_add(1);
		}
	});

	while (true) {
		auto total = size[0] * size[1] * spp;
		auto count = atomic_count.load();
		if (count < total) {
			std::printf("\r");
			std::printf("%f", 1.f * count / total);
		} else {
			future.wait();
			break;
		}
	}

	camera.to_path("build/test.exr");
	return 0;
}
