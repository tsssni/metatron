#pragma once
#include <metatron/core/math/filter/filter.hpp>

namespace metatron::math {
	struct Box_Filter final: Filter {
		Box_Filter(math::Vector<f32, 2> const& radius = {0.5f});

		auto operator()(math::Vector<f32, 2> const& p) const -> f32;
		auto sample(math::Vector<f32, 2> const& u) const -> std::optional<filter::Interaction>;

	private:
		math::Vector<f32, 2> radius;
	};
}
