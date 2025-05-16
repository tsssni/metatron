#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/sphere.hpp>

namespace metatron::math {
	struct Hemisphere_Distribution final {
		Hemisphere_Distribution() = default;

		auto sample(Vector<f32, 2> const& u) const -> Vector<f32, 3> {
			auto z = u[0];
			auto r = math::sqrt(1 - z * z);
			auto phi = 2.f * pi * u[1];
			return {r * std::cosf(phi), r * std::sin(phi), z};
		}

		auto pdf() const -> f32 {
			return 1.f / (2.f * pi);
		}
	};
}
