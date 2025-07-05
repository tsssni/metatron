#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::photo {
	namespace lens {
		struct Interaction final {
			math::Ray r;
			f32 pdf;
		};
	}

	struct Lens {
		virtual ~Lens() {}
		auto virtual sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> std::optional<lens::Interaction> = 0;
	};
}
