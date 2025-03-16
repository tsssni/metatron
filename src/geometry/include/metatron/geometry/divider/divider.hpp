#pragma once
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/geometry/material/material.hpp>
#include <metatron/volume/media/medium.hpp>

namespace metatron::divider {
	struct Divider final {
		shape::Shape const* shape;
		material::Material const* material;
		media::Medium const* interior_medium;
		media::Medium const* exterior_medium;
	};
}
