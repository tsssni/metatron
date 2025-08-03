#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/vector.hpp>
#include <variant>

namespace mtt::compo {
	struct Constant_Spectrum_Texture final {
		ecs::Entity spectrum;
	};

	struct Image_Spectrum_Texture final {
		std::string path;
		color::Color_Space::Spectrum_Type type;
	};

	using Spectrum_Texture = std::variant<
		Constant_Spectrum_Texture,
		Image_Spectrum_Texture
	>;

	struct Constant_Vector_Texture final {
		math::Vector<f32, 4> x;
	};

	struct Image_Vector_Texture final {
		std::string path;
	};

	using Vector_Texture = std::variant<
		Constant_Vector_Texture,
		Image_Vector_Texture
	>;

	using Texture = std::variant<
		Spectrum_Texture,
		Vector_Texture
	>;
}
