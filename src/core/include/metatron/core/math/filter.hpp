#pragma once
#include <metatron/core/math/vector.hpp>

namespace metatron::math {
	struct Filter {
		struct Sample final {
			math::Vector<f32, 2> p;
			f32 pdf;
		};
		auto virtual operator()(math::Vector<f32, 2> const& p) const -> f32 = 0;
		auto virtual sample(math::Vector<f32, 2> const& u) const -> math::Vector<f32, 2> = 0;
	};
}
