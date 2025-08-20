#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/compo/spectrum.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/vector.hpp>
#include <variant>

namespace mtt::compo {
	struct Constant_Spectrum_Texture final {
		ecs::Entity spectrum;
		i32 constant_spectrum{0};
	};

	struct Image_Spectrum_Texture final {
		std::string path;
		color::Color_Space::Spectrum_Type type;
		i32 image_spectrum{0};
	};

	struct Checkerboard_Texture final {
		ecs::Entity x;
		ecs::Entity y;
		math::Vector<usize, 2> uv_scale{1uz};
		i32 checkerboard{0};
	};

	using Spectrum_Texture = std::variant<
		Constant_Spectrum_Texture,
		Image_Spectrum_Texture,
		Checkerboard_Texture
	>;

	struct Constant_Vector_Texture final {
		math::Vector<f32, 4> x;
		i32 constant_vector{0};
	};

	struct Image_Vector_Texture final {
		std::string path;
		i32 image_vector{0};
	};

	using Vector_Texture = std::variant<
		Constant_Vector_Texture,
		Image_Vector_Texture
	>;

	using Texture = std::variant<
		Constant_Spectrum_Texture,
		Image_Spectrum_Texture,
		Checkerboard_Texture,
		Constant_Vector_Texture,
		Image_Vector_Texture
	>;
}
