#pragma once
#include <metatron/geometry/shape/sphere.hpp>

namespace metatron::material {
	template<typename T>
	struct Texture {
		using Element = T;
		auto virtual operator()(math::Vector<f32, 2> const& uv) -> Element = 0;
		auto virtual sample(shape::Interaction const& intr) -> Element = 0;
	};
}
