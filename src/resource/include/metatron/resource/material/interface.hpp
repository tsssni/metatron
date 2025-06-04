#pragma once
#include <metatron/resource/material/material.hpp>

namespace metatron::material {
	struct Interface_Material final: Material {
		Interface_Material();

		auto sample(
			eval::Context const& ctx,
			texture::Coordinate const& coord
		) const -> std::optional<Interaction>;
	};

}
