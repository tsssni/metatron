#pragma once
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/volume/media/medium.hpp>
#include <metatron/render/material/bsdf/bsdf.hpp>

namespace metatron::divider {
	struct Divider final {
		shape::Shape const* shape;
		media::Medium const* medium;
		material::Bsdf const* bsdf;
	};
}
