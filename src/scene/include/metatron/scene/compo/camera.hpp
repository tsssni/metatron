#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::compo {
	struct Pinhole_Lens final {
		f32 focal_length;
		i32 pinhole{0};
	};
	struct Thin_Lens final {
		f32 aperture;
		f32 focal_length;
		f32 focus_distance;
		i32 thin{0};
	};
	using Lens = std::variant<
		Pinhole_Lens,
		Thin_Lens
	>;

	struct Independent_Sampler final {
		i32 independent{0};
	};
	struct Halton_Sampler final {
		i32 halton{0};
	};
	using Sampler = std::variant<
		Independent_Sampler,
		Halton_Sampler
	>;

	struct Box_Filter final {
		math::Vector<f32, 2> radius{0.5f};
		i32 box{0};
	};
	struct Gaussian_Filter final {
		math::Vector<f32, 2> radius{1.5f};
		f32 sigma{0.5f};
		i32 gaussian{0};
	};
	struct Lanczos_Filter final {
		math::Vector<f32, 2> radius{0.5f};
		f32 tau{3.f};
		i32 lanczos{0};
	};
	using Filter = std::variant<
		Box_Filter,
		Gaussian_Filter,
		Lanczos_Filter
	>;

	struct Camera final {
		math::Vector<f32, 2> film_size = {0.036f, 0.024f};
		math::Vector<usize, 2> image_size = {1280uz, 720uz};
		usize spp = 16uz;
		usize depth = 64uz;
		Lens lens = Thin_Lens{
			.aperture = 5.6f,
			.focal_length = 0.05f,
			.focus_distance = 10.f,
		};
		Sampler sampler = Halton_Sampler{};
		Filter filter = Lanczos_Filter{};
		ecs::Entity initial_medium = "/hierarchy/medium/vaccum"_et;
		ecs::Entity color_space = "/color-space/sRGB"_et;
	};

	struct Camera_Space final {
		math::Transform world_to_render;
		math::Transform render_to_camera;
	};
}
