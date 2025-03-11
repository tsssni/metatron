#pragma once
#include <metatron/core/math/ray.hpp>

namespace metatron::medium {
	struct Context final {
		math::Ray r;
	};

	struct Interaction final {
	};

	struct Medium {
		auto virtual sample(Context const& ctx, f32 u) -> Interaction = 0;
	};
}
