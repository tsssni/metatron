#pragma once
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/geometry/material/material.hpp>
#include <metatron/volume/media/medium.hpp>
#include <metatron/render/light/light.hpp>

namespace metatron::divider {
	struct Divider final {
		shape::Shape const* shape{nullptr};
		usize primitive{0uz};
		material::Material const* material{nullptr};
		media::Medium const* interior_medium{nullptr};
		media::Medium const* exterior_medium{nullptr};
		light::Light const* area_light{nullptr};
	};
}
