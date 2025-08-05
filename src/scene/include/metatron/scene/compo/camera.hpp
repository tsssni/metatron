#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::compo {
	struct Pinhole_Lens final {
		std::string_view pinhole = "";
		f32 focal_length;
	};
	struct Thin_Lens final {
		std::string_view thin = "";
		f32 aperture;
		f32 focal_length;
		f32 focus_distance;
	};
	using Lens = std::variant<
		Pinhole_Lens,
		Thin_Lens
	>;

	struct Independent_Sampler final {
		std::string_view independent = "";
	};
	struct Halton_Sampler final {
		std::string_view halton = "";
	};
	using Sampler = std::variant<
		Independent_Sampler,
		Halton_Sampler
	>;

	struct Box_Filter final {
		std::string_view box = "";
		math::Vector<f32, 2> radius{0.5f};
	};
	struct Gaussian_Filter final {
		std::string_view gaussian = "";
		math::Vector<f32, 2> radius{1.5f};
		f32 sigma{0.5f};
	};
	struct Lanczos_Filter final {
		std::string_view lanczos = "";
		math::Vector<f32, 2> radius{0.5f};
		f32 tau{3.f};
	};
	using Filter = std::variant<
		Box_Filter,
		Gaussian_Filter,
		Lanczos_Filter
	>;

	struct Camera final {
		math::Vector<f32, 2> film_size;
		math::Vector<usize, 2> image_size;
		usize spp;
		usize depth;
		Lens lens;
		Sampler sampler;
		Filter filter;
		ecs::Entity initial_medium;
		ecs::Entity color_space;
	};

	struct Camera_Space final {
		math::Transform world_to_render;
		math::Transform render_to_camera;
	};
}
