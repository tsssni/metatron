#pragma once
#include <metatron/core/math/filter/filter.hpp>

namespace mtt::math {
	struct Box_Filter final: Filter {
		Box_Filter(Vector<f32, 2> const& radius = {0.5f});

		auto operator()(Vector<f32, 2> const& p) const -> f32;
		auto sample(Vector<f32, 2> const& u) const -> std::optional<filter::Interaction>;

	private:
		Vector<f32, 2> radius;
	};
}
