#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/sphere.hpp>

namespace metatron::math {
	struct Cone_Distribution final {
		Cone_Distribution(f32 cos_theta_max): cos_theta_max(cos_theta_max) {}

		auto sample(Vector<f32, 2> const& u) const -> Vector<f32, 3> {
			auto cos_theta = 1.f - u[0] * (1.f - cos_theta_max);
			auto phi = u[1] * 2.f * pi;
			return sphere_to_cartesion(cos_theta, phi);
		}

		auto pdf() const -> f32 {
			return 1.f / (2.f * pi * (1.f - cos_theta_max));
		};

	private:
		f32 cos_theta_max;
	};
}
