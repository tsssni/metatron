#pragma once
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/geometry/material/texture/texture.hpp>
#include <metatron/geometry/material/bsdf/bsdf.hpp>

namespace metatron::material {
	struct Material {
		auto virtual sample(Context const& ctx) const -> std::optional<std::unique_ptr<Bsdf>> = 0;
	};
}
