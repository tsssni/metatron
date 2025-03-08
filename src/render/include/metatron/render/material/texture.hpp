#pragma once
#include <metatron/geometry/intr/interaction.hpp>

namespace metatron::material {
	template<typename T>
	struct Texture {
		auto virtual operator[](math::Vector<f32, 2> const& uv) -> T = 0;
		auto virtual operator()(intr::Interaction const& intr) -> T = 0;
	};
}
