#pragma once
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/geometry/material/bsdf/bsdf.hpp>
#include <metatron/volume/media/medium.hpp>

namespace metatron::divider {
	struct Divider final {
		shape::Shape const* shape;
		material::Bsdf const* bsdf;
		media::Medium const* interior_medium;
		media::Medium const* exterior_medium;
	};
}
