#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/vector.hpp>
#include <glaze/glaze.hpp>

namespace mtt::compo {
	struct Constant_Spectrum final {
		f32 x;
		i32 constant{0};
	};

	struct Rgb_Spectrum final {
		math::Vector<f32, 3> c;
		color::Color_Space::Spectrum_Type type;
		ecs::Entity color_space;
		i32 rgb{0};
	};

	using Spectrum = std::variant<Constant_Spectrum, Rgb_Spectrum>;
}

template<>
struct glz::meta<mtt::color::Color_Space::Spectrum_Type> {
	using enum mtt::color::Color_Space::Spectrum_Type;
	auto constexpr static value = glz::enumerate(
		albedo,
		unbounded,
		illuminant
	);
};
