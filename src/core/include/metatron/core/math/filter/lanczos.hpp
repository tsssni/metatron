#pragma once
#include <metatron/core/math/filter/filter.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>
#include <metatron/core/math/trigonometric.hpp>

namespace mtt::math {
	struct Lanczos_Filter final {
		Lanczos_Filter(Vector<f32, 2> const& radius = {0.5f}, f32 tau = 3.f) noexcept;

		auto operator()(Vector<f32, 2> const& p) const noexcept -> f32;
		auto sample(Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction>;

	private:
		Piecewise_Distribution<64, 64> distribution;
		Vector<f32, 2> radius;
		f32 tau;
	};
}
