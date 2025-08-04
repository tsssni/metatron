#pragma once
#include <metatron/core/math/filter/filter.hpp>

namespace mtt::math {
	struct Box_Filter final {
		Box_Filter(Vector<f32, 2> const& radius = {0.5f}) noexcept;

		auto operator()(Vector<f32, 2> const& p) const noexcept -> f32;
		auto sample(Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction>;

	private:
		Vector<f32, 2> radius;
	};
}
