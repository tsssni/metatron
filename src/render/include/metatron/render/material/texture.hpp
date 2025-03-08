#pragma once
#include <metatron/geometry/intr/interaction.hpp>

namespace metatron::material {
	template<typename T>
	struct Texture {
		auto virtual operator()(intr::Interaction const& intr) -> T = 0;
	};
}
