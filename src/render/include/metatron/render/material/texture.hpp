#pragma once
#include <metatron/geometry/shape/sphere.hpp>

namespace metatron::material {
	template<typename T>
	struct Texture {
		auto virtual operator[](math::Vector<f32, 2> const& uv) -> T = 0;
		auto virtual operator()(shape::Interaction const& intr) -> T = 0;
	};
}
