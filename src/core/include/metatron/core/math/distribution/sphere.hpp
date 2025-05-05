#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/sphere.hpp>

namespace metatron::math {
	struct Sphere_Distribution final {
		Sphere_Distribution() = default;

		auto sample(Vector<f32, 2> const& u) const -> Vector<f32, 3> {
			auto cos_theta = 1.f - 2.f * u[0];
			auto sin_theta = std::sqrt(1 - cos_theta * cos_theta);
			auto phi = 2.f * pi * u[1];
			return {sin_theta * std::cosf(phi), cos_theta, sin_theta * std::sin(phi)};
		}

		auto pdf() const -> f32 {
			return 1.f / (4.f * pi);
		}
	};
}
