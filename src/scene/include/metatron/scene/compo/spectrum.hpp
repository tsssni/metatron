#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::compo {
	struct Constant_Spectrum final {
		std::string_view constant = "";
		f32 x;
	};

	struct Rgb_Spectrum final {
		std::string_view rgb = "";
		math::Vector<f32, 3> c;
		color::Color_Space::Spectrum_Type type;
		ecs::Entity color_space;
	};

	using Spectrum = std::variant<Constant_Spectrum, Rgb_Spectrum>;
}
